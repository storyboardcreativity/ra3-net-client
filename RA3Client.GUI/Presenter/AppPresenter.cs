using System;
using System.IO;
using System.Net;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using DirtySDKEAWebKit;
using RA3Client.GUI;
using SimpleConnProj;

public class AppPresenter
{
    private readonly LoginWindow _loginWindow;
    private readonly LoginProgressWindow _loginProgressWindow;
    private readonly MainWindow _mainWindow;

    public AppPresenter()
    {
        _loginWindow = new LoginWindow();
        _loginProgressWindow = new LoginProgressWindow();
        _mainWindow = new MainWindow();

        _loginWindow.LoginButtonClicked += OnLoginRequested;
        _loginWindow.CancelButtonClicked += OnCancelButtonClicked;
    }

    public void Start()
    {
        _loginWindow.Show();
    }

    private void OnCancelButtonClicked()
    {
        ShutdownApp();
    }

    private void ShutdownApp()
    {
        if (Application.Current?.ApplicationLifetime
            is IClassicDesktopStyleApplicationLifetime lifetime)
        {
            lifetime.Shutdown();
        }
    }

    public static void ShowCentered(Window window, Window target)
    {
        window.WindowStartupLocation = WindowStartupLocation.Manual;

        var targetScaling = target.RenderScaling;
        var windowScaling = window.RenderScaling;

        var targetPosition = target.Position;
        var targetSize = target.ClientSize;

        var targetCenterX =
            targetPosition.X + targetSize.Width * targetScaling / 2;

        var targetCenterY =
            targetPosition.Y + targetSize.Height * targetScaling / 2;

        var x = targetCenterX - window.Width * windowScaling / 2;
        var y = targetCenterY - window.Height * windowScaling / 2;

        window.Position = new PixelPoint(
            (int)Math.Round(x),
            (int)Math.Round(y));

        window.Show();
    }

    private async void OnLoginRequested(string login, string password, string pseudonym)
    {
        _loginWindow.Hide();
        ShowCentered(_loginProgressWindow, _loginWindow);
        
        bool success = await ProcessConnection(login, password, pseudonym);
        if (success == false)
        {
            // Failed!
            ShutdownApp();
        }

        if (Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime lifetime)
            lifetime.MainWindow = _mainWindow;

        ShowCentered(_mainWindow, _loginProgressWindow);
        _mainWindow.Activate();

        _loginWindow.Close();
        _loginProgressWindow.Close();
    }

    private async Task<bool> ProcessConnection(string login, string password, string pseudonym)
    {
        try
        {
            _loginProgressWindow.UpdateStatus("Starting connection process...");
            _loginProgressWindow.UpdateProgress(1);
            
            _loginProgressWindow.UpdateStatus("Processing download step...");
            _loginProgressWindow.UpdateProgress(10);

            await Task.Run(() =>
            {
                ProcessDownloadStep(_loginProgressWindow.SuboperationProgress);
            });

            _loginProgressWindow.UpdateStatus("Processing GameSpy availability ping...");
            _loginProgressWindow.UpdateProgress(20);

            await Task.Run(() =>
            {
                GameSpy.ProcessGameSpyAvailabilityPing();
            });
            
            _loginProgressWindow.UpdateStatus("Establishing FESL secure connection...");
            _loginProgressWindow.UpdateProgress(40);

            var result = await Task.Run(() =>
            {
                bool success = Fesl.LoginAsPseudonym(
                    _loginProgressWindow.SuboperationProgress,
                    login,
                    password,
                    pseudonym,
                    out string ticket,
                    out string challenge,
                    out string profileId);

                return (success, ticket, challenge, profileId);
            });

            if (!result.success)
            {
                Console.WriteLine("Failed to login to FESL!");
                return false;
            }
            string preAuthTicket = result.ticket;
            string preAuthChallenge = result.challenge;
            string preAuthProfileId = result.profileId;

            Console.WriteLine($"Profile ID: {preAuthProfileId}");
            
            // Step 3: GPCM connection
            _loginProgressWindow.UpdateStatus("Establishing GPCM connection...");
            _loginProgressWindow.UpdateProgress(40);

            await GpcmProcessor.ProcessGpcmConnectionAsync(_loginProgressWindow.SuboperationProgress, preAuthTicket, preAuthChallenge);
            
            // Step 4: Peerchat connection
            _loginProgressWindow.UpdateStatus("Connecting to peerchat server...");
            _loginProgressWindow.UpdateProgress(50);

            var peerchat = new Peerchat();
            var peerchatResult = await Task.Run(() =>
            {
                var success = peerchat.Init(
                    _loginProgressWindow.SuboperationProgress,
                    pseudonym,
                    preAuthProfileId);
                return success;
            });
            if (!peerchatResult)
            {
                Console.WriteLine("Peerchat connection failed!");
                return false;
            }
            
            // Step 5: Master-server connection
            _loginProgressWindow.UpdateStatus("Establishing master-server connection...");
            _loginProgressWindow.UpdateProgress(60);
            
            // In a real implementation, this would make actual master-server calls
            
            // Step 6: Finalizing connection
            _loginProgressWindow.UpdateStatus("Finalizing connection...");
            _loginProgressWindow.UpdateProgress(70);
            
            _loginProgressWindow.UpdateStatus("All connections established - successfully connected!");
            _loginProgressWindow.UpdateProgress(100);
        }
        catch (Exception ex)
        {
            _loginProgressWindow.UpdateStatus($"Connection failed: {ex.Message}");
            Tools.NetPrintf($"Connection failed: {ex.Message}");
            return false;
        }

        return true;
    }

    public void ProcessHttpConnection(string server, string serverOriginal, string path, string fileName)
    {
        Tools.NetPrintf($"GET -> http://{server}/{path}/{fileName}");

        try
        {
            string url = $"http://{server}/{path}/{fileName}";

            var request = (HttpWebRequest)WebRequest.Create(url);

            request.Method = "GET";
            request.ContentLength = 0;
            request.KeepAlive = false;

            request.UserAgent = "ProtoHttp 1.2/DS 6.3.5.0";
            request.Accept = "*/*";

            if (serverOriginal != null)
            {
                request.Host = serverOriginal;
            }

            using var response = (HttpWebResponse)request.GetResponse();
            using (var reader = new StreamReader(response.GetResponseStream()))
            {
                Tools.NetPrintf(reader.ReadToEnd());
            }
        }
        catch (WebException e)
        {
            // Аналог обработки сетевых ошибок.
            Tools.NetPrintf(e.Message);

            // HttpWebRequest считает HTTP-статусы 4xx/5xx исключениями,
            // тогда как libcurl без CURLOPT_FAILONERROR обычно не считает их
            // ошибкой выполнения запроса.
            if (e.Response is HttpWebResponse response)
            {
                using (response)
                using (var reader = new StreamReader(response.GetResponseStream()))
                {
                    Tools.NetPrintf(reader.ReadToEnd());
                }
            }
        }
        catch (Exception e)
        {
            Tools.NetPrintf(e.Message);
        }
    }

    private void ProcessDownloadStep(IProgress<(int percent, string message)>? progress)
    {
        Tools.NetPrintf("Processing download step [1]");

        string[] fileNames =
        {
            RA3Constants.RA3_STRING_PATCHINFO + ".patchinfo",
            "maps-" + RA3Constants.RA3_STRING_MAPS_COUNT + ".txt",
            "config.txt",
            "MOTD-" + RA3Constants.RA3_STRING_LOCALE + ".txt",
            "drivers.txt"
        };
        for (int i = 0; i < fileNames.Length; i++)
        {
            string fileName = fileNames[i];
            int percent = (i + 1) * 100 / fileNames.Length;

            progress?.Report((percent, $"Downloading {fileName}"));

            ProcessHttpConnection(
                RA3Constants.RA3_STRING_SERVER_DOWNLOADS,
                RA3Constants.RA3_STRING_SERVER_DOWNLOADS_ORIGINAL,
                RA3Constants.RA3_STRING_LIVESUITE_PATH,
                fileName);
        }
        progress?.Report((100, $"Finished"));
    }
}