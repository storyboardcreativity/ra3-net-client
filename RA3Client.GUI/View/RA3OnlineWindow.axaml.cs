using Avalonia.Controls;
using System;

namespace RA3Client.GUI;

public partial class RA3OnlineWindow : Window
{
    public event Action? LogoutButtonClicked;

    public RA3OnlineWindow()
    {
        InitializeComponent();
    }

    private void LogoutButton_Click(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        LogoutButtonClicked?.Invoke();
    }
}