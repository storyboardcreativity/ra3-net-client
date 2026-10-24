using System;

public sealed class Ra3ClientInfo
{
    private readonly object _mutex = new();

    private string _preauthTicket = "";
    private string _preauthChallenge = "";
    private string _gpcmInitialChallenge = "";
    private string _gpcmOurChallenge = "";
    private string _nuloginpersonaProfileId = "";

    private string Get(ref string field)
    {
        lock (_mutex)
            return field;
    }

    private void Set(ref string field, string value)
    {
        lock (_mutex)
            field = value;
    }

    public string PreAuthTicket
    {
        get => Get(ref _preauthTicket);
        set => Set(ref _preauthTicket, value);
    }
    public string PreAuthChallenge
    {
        get => Get(ref _preauthChallenge);
        set => Set(ref _preauthChallenge, value);
    }
    public string GpcmInitialChallenge
    {
        get => Get(ref _gpcmInitialChallenge);
        set => Set(ref _gpcmInitialChallenge, value);
    }
    public string GpcmOurChallenge
    {
        get => Get(ref _gpcmOurChallenge);
        set => Set(ref _gpcmOurChallenge, value);
    }
    public string NuLoginPersonaProfileId
    {
        get => Get(ref _nuloginpersonaProfileId);
        set => Set(ref _nuloginpersonaProfileId, value);
    }
}