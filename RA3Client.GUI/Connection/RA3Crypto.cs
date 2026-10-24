using System;
using System.Security.Cryptography;
using System.Text;

public static class RA3Crypto
{
    public static string ConvertToHexString(string str)
    {
        return Convert.ToHexString(Encoding.UTF8.GetBytes(str)).ToLowerInvariant();
    }

    public static string Md5(string str)
    {
        byte[] hash = MD5.HashData(Encoding.UTF8.GetBytes(str));
        return Convert.ToHexString(hash).ToLowerInvariant();
    }

    public static string GetGpcmChallengeResponseMagic(
        string preAuthTicket,
        string preAuthChallenge,
        string ourChallenge,
        string initialChallenge)
    {
        return Md5(ConvertToHexString(preAuthChallenge))
             + "                                                "
             + preAuthTicket
             + ourChallenge
             + initialChallenge
             + Md5(ConvertToHexString(preAuthChallenge));
    }

    public static string GetGpcmChallengeResponse(
        string preAuthTicket,
        string preAuthChallenge,
        string ourChallenge,
        string initialChallenge)
    {
        return Md5(ConvertToHexString(GetGpcmChallengeResponseMagic(preAuthTicket, preAuthChallenge, ourChallenge, initialChallenge)));
    }
}