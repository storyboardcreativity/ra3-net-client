using System;
using System.Text;
using System.Diagnostics;
using System.Buffers.Binary;
using DirtySDKEAWebKit;
using DirtySDKEAWebKit.CSharp;
using System.Text.RegularExpressions;
using System.Linq;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using System.Threading;

namespace SimpleConnProj;

public static class Fesl
{
    public static bool LoginAsPseudonym(
        IProgress<(int percent, string message)>? progress,
        string login,
        string password,
        string pseudonym,
        out string ticket,
        out string challenge,
        out string profileId)
    {
        ticket = "";
        challenge = "";
        profileId = "";

        progress?.Report((1, "Connecting to FESL"));

        var client = new FeslClient();
        client.Initialize();

        progress?.Report((10, "Logging in to FESL"));

        // Login to FESL
        if (client.Login(login, password) == false)
            return false;

        progress?.Report((30, "Setting our nick in FESL as a pseudonym"));

        // Get personas (TODO: use this info to NOT add persona if exists)
        var personas = client.Personas();

        progress?.Report((50, "Setting our nick in FESL as a pseudonym"));

        // Add our persona
        client.AddPersona(pseudonym);

        progress?.Report((60, "Setting our nick in FESL as a pseudonym"));

        // Login to our persona
        if (!client.LoginPersona(pseudonym, out profileId))
            return false;
        
        //client.GetTelemetryToken();
        //client.ProcessPing();

        progress?.Report((80, "Getting FESL pre-auth ticket and challenge"));

        if (client.GameSpyPreAuth(out ticket, out challenge) == false)
            return false;

        progress?.Report((90, "Closing FESL connection"));

        // Shutdown the connection
        client.DeInitialize();

        progress?.Report((100, "Finished"));

        return true;
    }
}