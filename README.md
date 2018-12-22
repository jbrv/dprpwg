# dprpwg: a Deterministic Pseudo-Random PassWord Generator

`dprpwg` is a Deterministic Pseudo-Random Password Generator.
**It does not store a password database.**
It is intended to be used for website accounts.

Everyone know good practices for passwords on the web:
- Use a combinaison of letters (upper and lower case), digits, and symbols;
- Have a long-enough password;
- Use one password per website;
- Change passwords often;
- <One more another annoying advice that you won't follow>;
- etc...

This tool should help to fulfill at least the fourth first advices listed
above.

## How that works

To generate a password, this tool uses the following inputs:
- One **master password**. The one you always use, for example ;)
- One **domain name**, so you can have one password per website;
- The **year**, to encourage you to change the password every year;
- The **symbol category selection** to use in the generated password
(upper case, lower case letters, digits, symbols)

With these inputs, it generates a **deterministic, pseudo-random password,**
containing (should) at least one symbol from each symbol category.

*Deterministic*, because given the same input combinaison, it generates
the same password.

*Pseudo-random*, because it uses the input like a hash algorithm would do.
**BIG DISCLAIMER, by the way: this is not a sha-2 hash or something!**
This is a probably very bad, home-designed hash algorithm. I am not sure
it would hold long against a cryptographic analysis.
But hey, I'm no cryptanalist either.

The main feature here: **it does not generate/store any password database!**
To retrieve the password you registered with on any website, just input
the website, the year and your master password, and you get it.

Hopefully, the base password should be very hard to find, given the generated
password. So, if your account is hacked in some website (and if this tool
is not so commonly used...), the disater should be contained.

## Building

### Generate `dprpwg_config.h`

TODO

### Now, proper building

TODO

## Using

TODO

## License

This tool is licensed under the MIT License - see the COPYING file for details
