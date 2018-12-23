/*
 * dprpwg: a Deterministic Pseudo-Random PassWord Generator
 * Copyright (c) 2018 Jean-Baptiste HERVE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef DPRPWG_LIB_H
#define DPRPWG_LIB_H

#include <stddef.h> /* For size_t definition */

/* Symbol categories */
#define OUTPUT_LOW "azertyuiopqsdfghjklmwxcvbn"   /* Lower case letters */
#define OUTPUT_UPP "FGHJKLMWXCVBNAZERTYUIOPQSD"   /* Upper case letters */
#define OUTPUT_DIG "0123456789"                   /* Digits */
#define OUTPUT_SYM "()[]-_{}=+!:/;.,?"            /* Symbols */

/* Available symbol list, max length. 256 = ASCII table length */
#define OUTPUT_DOMAIN_MAXLENGTH   256U

/* Min and max password length (ignored if manual length is required) */
#define OUTPUT_MIN_LENGTH 12U
#define OUTPUT_MAX_LENGTH 256U

/* Output symbol configuration flags */
#define FLAG_LOW_AVAIL  (1U<<0)
#define FLAG_UPP_AVAIL  (1U<<1)
#define FLAG_DIG_AVAIL  (1U<<2)
#define FLAG_SYM_AVAIL  (1U<<3)

/* Maximum number of iteration to find a correct password, containing
 * all the required symbol types */
#define ITERATION_MAX 65536

/* Completery arbitrary value for a "impossible to crack" password strength */
#define OVERKILL_PWD_STRENGTH 30.0

/* That would be in "glib", won't include it for that */
#ifndef FALSE
#  define FALSE 0
#endif
#ifndef TRUE
#  define TRUE 1
#endif

/**
 * \brief Password generation function
 * \param password  Base, master password, that must be remembered.
 * \param domain    Domain name where the password is to be used.
 * \param year      Year, so people are incitated to change password every year.
 * \param fixed_size  Fixed password size. Ignored if <= 0.
 * \param new_passwd  Pointer to a new null-terminated string that will
 *                    contain the new password. Allocated by this function,
 *                    to be given to free() (and maybe to memset() before)
 *                    when not used any more.
 * \param flags     Flags to select which symbol categories to use. or'ed
 *                  combinaison of FLAG_LOW_AVAIL, FLAG_UPP_AVAIL, FLAG_DIG_AVAIL
 *                  and FLAG_SYM_AVAIL.
 *
 * This function generates a deterministic, pseudo-random password according
 * to the given inputs. Only the base password should be remembered.
 * Nothing is saved by this function.
 *
 * The generated password should be unique, given one combinaison of the
 * inputs. It should also be hard to find the base password, given the
 * generated password. So, even if your account is hacked in one website,
 * the disaster should be contained.
 *
 * If not fixed, generated password length depends on the year. It will
 * increase, at the rate of one character every 5 year. It may compensate
 * for the increasing computing power of hackers.
 *
 * *new_passwd will be allocated by this function. Give it to free() when
 * you don't need it anymore. Oh, and probably to memset() just before,
 * maybe you don't want to have a password somewhere in memory...
 */
void generate_password(const char   *password,
                       const char   *domain,
                       const char   *year,
                       size_t       fixed_size,
                       char         **new_passwd,
                       unsigned int flags);

/**
 * \brief Password strength computation
 * \param password  The password
 * \param year      Year the password will be used
 * \param flags     Flags to select which symbol categories are used. or'ed
 *                  combinaison of FLAG_LOW_AVAIL, FLAG_UPP_AVAIL, FLAG_DIG_AVAIL
 *                  and FLAG_SYM_AVAIL.
 * \return A decimal number giving the password strength.
 *
 * The password strength value has an abritrary unit. The password should
 * be considered too weak if the value is lower than 0.5, and excellent
 * if the value is higher than 0.9.
 */
double get_password_strength(const char* password, unsigned int year, unsigned int flags);

#endif /* DPRPWG_LIB_H */
