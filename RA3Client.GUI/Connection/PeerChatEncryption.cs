
using System;

public class PeerChatEncryption
{
    public byte GsPeerChat1 = 0;
    public byte GsPeerChat2 = 0;
    public byte[] GsPeerChatCrypt = new byte[256];

    public void GsPeerChatInit(byte[] chall, byte[] gamekey)
    {
        if (chall == null)
            throw new ArgumentNullException(nameof(chall));

        if (gamekey == null)
            throw new ArgumentNullException(nameof(gamekey));

        GsPeerChat1 = 0;
        GsPeerChat2 = 0;

        byte[] challenge = new byte[16];
        byte[] crypt = GsPeerChatCrypt;

        int p1 = 0;

        // Equivalent to the original C challenge-generation loop.
        for (int i = 0; i < 16; i++)
        {
            if (gamekey[p1] == 0)
                p1 = 0;

            challenge[i] = (byte)(chall[i] ^ gamekey[p1]);
            p1++;
        }

        // Initialize the permutation in descending order: 255..0.
        byte t1 = 255;

        for (int i = 0; i < 256; i++)
        {
            crypt[i] = t1;
            t1 = unchecked((byte)(t1 - 1));
        }

        // Equivalent to the original KSA loop.
        t1 = unchecked((byte)(t1 + 1));

        int p = 0;
        p1 = 0;

        for (int i = 0; i < 256; i++)
        {
            t1 = unchecked((byte)(
                t1 + challenge[p1] + crypt[p]));

            byte t = crypt[t1];
            crypt[t1] = crypt[p];
            crypt[p] = t;

            p++;
            p1++;

            if (p1 == 16)
                p1 = 0;
        }

        GsPeerChatCrypt = crypt;
    }

    public void GsPeerChat(byte[] data, int size)
    {
        if (data == null)
            throw new ArgumentNullException(nameof(data));

        if (size < 0 || size > data.Length)
            throw new ArgumentOutOfRangeException(nameof(size));

        byte num1 = GsPeerChat1;
        byte num2 = GsPeerChat2;
        byte[] crypt = GsPeerChatCrypt;

        int offset = 0;

        while (size-- > 0)
        {
            num1 = unchecked((byte)(num1 + 1));

            byte t = crypt[num1];

            num2 = unchecked((byte)(num2 + t));

            crypt[num1] = crypt[num2];
            crypt[num2] = t;

            t = unchecked((byte)(t + crypt[num1]));

            data[offset++] ^= crypt[t];
        }

        GsPeerChat1 = num1;
        GsPeerChat2 = num2;
        GsPeerChatCrypt = crypt;
    }
}
