using Avalonia.Controls;
using System;
using System.Text;
using System.Threading.Tasks;
using System.Linq;
using System.IO;
using System.Net;
using DirtySDKEAWebKit;
using SimpleConnProj;

namespace RA3Client.GUI;

public partial class MainWindow : Window
{
    private LoginProgressWindow? loginProgressWindow;
    private RA3OnlineWindow? ra3OnlineWindow;
    private bool isConnected = false;
    private StringBuilder logBuffer = new StringBuilder();

    public MainWindow()
    {
        InitializeComponent();
        SetupEventHandlers();
        LogMessage("RA3 Client GUI initialized");
    }

    private void SetupEventHandlers()
    {
        //ConnectButton.Click += ConnectButton_Click;
        //DisconnectButton.Click += DisconnectButton_Click;
        //ExitButton.Click += ExitButton_Click;
    }

    private void ShowOnlineWindow()
    {
        ra3OnlineWindow = new RA3OnlineWindow();
        ra3OnlineWindow.Show();
        this.Hide();
        
        isConnected = true;
        UpdateUI();
        LogMessage("Successfully connected to RA3 server!");
    }

    private void UpdateUI()
    {
        StatusText.Text = isConnected ? "Connected" : "Not connected";
        ServerText.Text = isConnected ? "Server: ra3-server.example.com" : "Server: Not set";
        //UsernameText.Text = isConnected ? $"User: {currentPseudonym}" : "User: Not set";
        ConnectButton.IsEnabled = !isConnected;
        DisconnectButton.IsEnabled = isConnected;
    }

    private void LogMessage(string message)
    {
        var timestamp = DateTime.Now.ToString("HH:mm:ss");
        var logEntry = $"[{timestamp}] {message}{Environment.NewLine}";
        logBuffer.Append(logEntry);
        
        // Keep only the last 100 lines to prevent memory issues
        var lines = logBuffer.ToString().Split(new[] { Environment.NewLine }, StringSplitOptions.RemoveEmptyEntries);
        if (lines.Length > 100)
        {
            // Use ArraySegment to get last 100 items
            var lastLines = lines.Skip(Math.Max(0, lines.Length - 100)).ToArray();
            logBuffer.Clear();
            foreach (var line in lastLines)
            {
                logBuffer.AppendLine(line);
            }
        }
        
        LogText.Text = logBuffer.ToString();
    }
}