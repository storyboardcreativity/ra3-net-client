using Avalonia.Controls;
using System;

namespace RA3Client.GUI;

public partial class LoginWindow : Window
{
    public event Action<string, string, string>? LoginButtonClicked;
    public event Action? CancelButtonClicked;

    private bool _isDarkTheme;

    public LoginWindow()
    {
        InitializeComponent();
        SetupEventHandlers();
        
        // Set default login credentials for testing purposes
        SetLogin("abcdefg@ra3.cnc");
        SetPassword("abcdefg");
        SetUserId("abcdefg");
    }

    private void SetupEventHandlers()
    {
        LoginButton.Click += LoginButton_Click;
        CancelButton.Click += CancelButton_Click;
    }

    private void LoginButton_Click(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        LoginButtonClicked?.Invoke(
            LoginTextBox.Text,
            PasswordTextBox.Text,
            UserIdTextBox.Text);
    }

    private void CancelButton_Click(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        CancelButtonClicked?.Invoke();
    }

    private void ThemeButton_Click(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        _isDarkTheme = !_isDarkTheme;

        if (_isDarkTheme)
        {
            Classes.Remove("light");
            Classes.Add("dark");
            ThemeButton.Content = "☀️";
            ToolTip.SetTip(ThemeButton, "Переключить на светлую тему");
        }
        else
        {
            Classes.Remove("dark");
            Classes.Add("light");
            ThemeButton.Content = "🌙";
            ToolTip.SetTip(ThemeButton, "Переключить на тёмную тему");
        }
    }

    public void SetRememberMe(bool checkedState)
    {
        RememberMeCheckBox.IsChecked = checkedState;
    }

    public bool GetRememberMe()
    {
        return RememberMeCheckBox.IsChecked ?? false;
    }

    public string GetLogin()
    {
        return LoginTextBox.Text;
    }

    public string GetPassword()
    {
        return PasswordTextBox.Text;
    }

    public string GetUserId()
    {
        return UserIdTextBox.Text;
    }

    public void SetLogin(string login)
    {
        LoginTextBox.Text = login;
    }

    public void SetPassword(string password)
    {
        PasswordTextBox.Text = password;
    }

    public void SetUserId(string userId)
    {
        UserIdTextBox.Text = userId;
    }
}