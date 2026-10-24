using System;

public sealed class Peerchat
{
    private PeerchatConnection _connection;
    //private readonly Func<string, string> _getProfileId;

    public bool Init(
        IProgress<(int percent, string message)>? progress,
        string pseudonym,
        string profileId)
    {
        progress?.Report((0, "Starting peerchat module"));

        var peerchatConnection = new PeerchatConnection();

        progress?.Report((1, "Initializing peerchat connection"));

        if (!peerchatConnection.Init(RA3Constants.RA3_STRING_SERVER_PEERCHAT, RA3Constants.RA3_STRING_SERVER_PEERCHAT_PORT))
            return false;

        progress?.Report((20, "Sending game name"));

        if (!peerchatConnection.SendCrypt(RA3Constants.RA3_STRING_GAMENAME, RA3Constants.RA3_STRING_PEERCHAT_SECRET_KEY))
            return false;

        progress?.Report((40, "Getting our IP from server"));

        peerchatConnection.SendUsrIp();

        progress?.Report((60, "Getting our IP from server"));

        if (!peerchatConnection.SendUser(profileId, pseudonym, RA3Constants.RA3_STRING_SERVER_PEERCHAT_ORIGINAL, RA3Constants.RA3_ACCOUNT_CDKEY_ENCODED))
            return false;

        progress?.Report((90, "Sending our CD key"));

        peerchatConnection.SendCdKey(RA3Constants.RA3_ACCOUNT_CDKEY);

        progress?.Report((100, "Finished"));

        return true;
    }
}
