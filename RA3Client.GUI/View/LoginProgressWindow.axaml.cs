using System;
using Avalonia.Controls;

namespace RA3Client.GUI;

public partial class LoginProgressWindow : Window
{
    public LoginProgressWindow()
    {
        InitializeComponent();
        SuboperationProgress = new Progress<(int percent, string message)>(UpdateSuboperation);
    }

    public IProgress<(int percent, string message)>? SuboperationProgress
    {
        get;
        set;
    }

    private void UpdateSuboperation((int percent, string message) progress)
    {
        SuboperationStatusText.Text = progress.message;
        SuboperationProgressBar.Value = Math.Clamp(progress.percent, 0, 100);
    }

    public void ResetSuboperation()
    {
        SuboperationStatusText.Text = string.Empty;
        SuboperationProgressBar.Value = 0;
    }

    public void UpdateStatus(string status)
    {
        StatusText.Text = status;
    }

    public void UpdateProgress(int progress)
    {
        ProgressBar.Value = progress;
    }

    public void SetCancelButtonVisibility(bool visible)
    {
        CancelButton.IsVisible = visible;
    }
}