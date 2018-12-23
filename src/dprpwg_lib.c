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

#include "dprpwg_lib.h"
#include "dprpwg_config.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* Some internal functions declarations */

/* Min and Max. Mostly used for size_t values */
static inline size_t max(size_t a, size_t b)
{
  return a > b ? a : b;
}

static inline size_t min(size_t a, size_t b)
{
  return a < b ? a : b;
}

/* Check if a password contains all the required symbol categories */
static int check_password(const char* password, unsigned int flags);

/* Check if a password contains one of the symbol of a given domain */
static int check_password_domain(const char* password, const char* domain);

/* The main function of this tool. Generate a password. */
void generate_password(const char   *password,
                       const char   *domain,
                       const char   *year,
                       size_t       fixed_size,
                       char         **new_passwd,
                       unsigned int flags)
{

  /* ---- Variable declarations ---- */
  /* This will be the symbol domain list and its size */
  char output_domain[OUTPUT_DOMAIN_MAXLENGTH];
  size_t output_domain_size;

  /* Aliases for various string lengths */
  size_t password_length, domain_length, year_length, output_length;

  /* Temporary hash used during generation */
  uint16_t* password_hash;

  /* Cursors needed when reading the inputs */
  size_t pwd_seek, domain_seek, year_seek, output_seek;

  /* Control the number of iteration */
  size_t limit, iteration;
  /* ---- End of variable declarations ---- */

  /* No symbol category selected? empty password, then */
  if (!flags) {
    *new_passwd = calloc(1, sizeof(char));
    return;
  }

  /* ---- Generate the available output symbol domain ---- */
  memset(output_domain, 0, OUTPUT_DOMAIN_MAXLENGTH);
  domain_seek = 0;

  /* For each symbol category: check if it is requested. If so, add the
   * symbol category list to the output symbol domain */
  if (flags & FLAG_LOW_AVAIL) {
    strncpy(output_domain, OUTPUT_LOW, max(strlen(OUTPUT_LOW), OUTPUT_DOMAIN_MAXLENGTH));
    domain_seek = strlen(OUTPUT_LOW);
  }

  if (flags & FLAG_DIG_AVAIL) {
    strncpy(output_domain + domain_seek, OUTPUT_DIG,
            max(strlen(OUTPUT_DIG), OUTPUT_DOMAIN_MAXLENGTH - domain_seek));
    domain_seek += strlen(OUTPUT_DIG);
  }

  if (flags & FLAG_SYM_AVAIL) {
    strncpy(output_domain + domain_seek, OUTPUT_SYM,
            max(strlen(OUTPUT_SYM), OUTPUT_DOMAIN_MAXLENGTH - domain_seek));
    domain_seek += strlen(OUTPUT_SYM);
  }

  if (flags & FLAG_UPP_AVAIL) {
    strncpy(output_domain + domain_seek, OUTPUT_UPP,
            max(strlen(OUTPUT_UPP), OUTPUT_DOMAIN_MAXLENGTH - domain_seek));
    domain_seek += strlen(OUTPUT_UPP);
  }

  /* ---- Output symbol domain generated ---- */

  /* Set the string length aliases. Yes, they could be 'const'... */
  password_length = strlen(password);
  domain_length = strlen(domain);
  year_length = strlen(year);
  output_domain_size = strlen(output_domain);

  /* Compute the length of the generated password if this is not fixed */
  if (fixed_size > 0) {
    output_length = fixed_size;
  } else {
    /*
     * Oh yeah, that's arbitrary. The size should be as follow:
     * - pre 2000:  12
     * - 2000-2004: 12
     * - 2005-2009: 13
     * - 2010-2014: 14
     * - 2015-2020: 15
     * ... and I thing you get it.
     */
    int year_value = atoi(year);

    if (year_value < 2000) {
      output_length = 12;
    } else {
      output_length = max(OUTPUT_MIN_LENGTH,
                          min(OUTPUT_MAX_LENGTH,
                              12 + (unsigned int)(year_value - 2000) / 5));
    }
  }

  /* Memory allocation for temporary hash and output password */
  password_hash = calloc(output_length, sizeof(uint16_t));
  *new_passwd = calloc(output_length + 1, sizeof(char));

  /* Initialize iteration count and string cursors */
  pwd_seek = domain_seek = year_seek = output_seek = 0;
  iteration = 0;

  /* Compute the number of iteration to use. Depends on the input */
  limit = output_domain_size * (password_length + domain_length + year_length + output_length + flags);

  /* One turn of the generation algorithm */
  while (iteration < limit) {
    /* First, check cursors and reset them if needed */
    if (pwd_seek >= password_length) {
      pwd_seek = 0;
    }

    if (domain_seek >= domain_length) {
      domain_seek = 0;
    }

    if (year_seek >= year_length) {
      year_seek = 0;
    }

    if (output_seek >= output_length) {
      output_seek = 0;
    }

    /* If we are given a password... */
    if (password_length) {
      /* Oh yeah... Do something with the password.
       * Look at the code! Splendid. Neat. Marvelous. */
      password_hash[output_seek] = (password_hash[output_seek]
                                    + ((unsigned int)(password[pwd_seek])) * PW_MUL
                                    + output_seek * pwd_seek * PW_SEEK_MUL
                                    + ((unsigned int)(password[password_length - pwd_seek - 1])) * PW_INV_MUL
                                   ) % 65536;
    }

    /* Use also the domain, ... */
    if (domain_length) {
      password_hash[output_seek] = (password_hash[output_seek]
                                    + ((unsigned int)(domain[domain_seek])) * DOM_MUL
                                    + output_seek * domain_seek * DOM_SEEK_MUL
                                    + ((unsigned int)(domain[domain_length - domain_seek - 1])) * DOM_INV_MUL
                                   ) % 65536;
    }

    /* ... and the year. */
    if (year_length) {
      password_hash[output_seek] = (password_hash[output_seek]
                                    + ((unsigned int)(year[year_seek])) * YR_MUL
                                    + output_seek * year_seek * YR_SEEK_MUL
                                    + ((unsigned int)(year[year_length - year_seek - 1])) * YR_INV_MUL
                                   ) % 65536;
    }

    /* Now we have a new character. Note that it may be modified until
     * the last loop iteration */
    (*new_passwd)[output_seek] = output_domain[password_hash[output_seek] % output_domain_size];

    /* Increment everything */
    output_seek++;
    pwd_seek++;
    year_seek++;
    domain_seek++;
    iteration++;

    /* Stop if we reach the limit AND we have all the requested symbol
     * categories in the password! If it lacks some categories, raise the
     * limit. */
    if ((iteration == limit && limit < ITERATION_MAX) && !check_password(*new_passwd, flags)) {
      /* Proceed again, but up to a certain point. Note that it means
       * the generated password may not contain all symbols. */
      limit += output_length;

      if (limit > ITERATION_MAX) {
        limit = ITERATION_MAX;
      }
    }
  }

  /* Some cleaning. Yes, do some memset() to avoid random data in ram */
  memset(output_domain, 0, OUTPUT_DOMAIN_MAXLENGTH * sizeof(char));
  memset(password_hash, 0, output_length * sizeof(uint16_t));
  free(password_hash);
}

/* Check the password contains all requested symbol categories */
static int check_password(const char* password, unsigned int flags)
{

  if (flags & FLAG_DIG_AVAIL) {
    if (!check_password_domain(password, OUTPUT_DIG)) {
      return FALSE;
    }
  }

  if (flags & FLAG_SYM_AVAIL) {
    if (!check_password_domain(password, OUTPUT_SYM)) {
      return FALSE;
    }
  }

  if (flags & FLAG_LOW_AVAIL) {
    if (!check_password_domain(password, OUTPUT_LOW)) {
      return FALSE;
    }
  }

  if (flags & FLAG_UPP_AVAIL) {
    if (!check_password_domain(password, OUTPUT_UPP)) {
      return FALSE;
    }
  }

  return TRUE;
}

/* Check if 'password' contains at least one symbol from 'domain' */
static int check_password_domain(const char* password, const char* domain)
{
  size_t password_length, password_seek, domain_length, domain_seek;

  password_length = strlen(password);
  domain_length = strlen(domain);

  for (password_seek = 0; password_seek < password_length; password_seek++) {
    for (domain_seek = 0; domain_seek < domain_length; domain_seek++) {
      if (password[password_seek] == domain[domain_seek]) {
        return TRUE;
      }
    }
  }

  return FALSE;
}


/* Compute a password strength */
double get_password_strength(const char* password, unsigned int year, unsigned int flags)
{
  int symbols_count_table[OUTPUT_DOMAIN_MAXLENGTH];
  double entropy = 0.0;
  size_t password_length;
  size_t table_seek, password_seek;
  size_t alphabet_size = 0;

  if (flags & FLAG_DIG_AVAIL) {
    alphabet_size += strlen(OUTPUT_DIG);
  }

  if (flags & FLAG_SYM_AVAIL) {
    alphabet_size += strlen(OUTPUT_SYM);
  }

  if (flags & FLAG_LOW_AVAIL) {
    alphabet_size += strlen(OUTPUT_LOW);
  }

  if (flags & FLAG_UPP_AVAIL) {
    alphabet_size += strlen(OUTPUT_UPP);
  }

  password_length = strlen(password);

  if (password_length <= 0) {
    return 0.0;
  }

  /* Table initialization */
  memset(symbols_count_table, 0, OUTPUT_DOMAIN_MAXLENGTH * sizeof(int));

  /* Scan the password and count the points */
  for (password_seek = 0; password_seek < password_length; password_seek++) {
    symbols_count_table[(unsigned int) password[password_seek]]++;
  }

  /* Compute the "Shannon entropy" of the password */
  for (table_seek = 0; table_seek < OUTPUT_DOMAIN_MAXLENGTH; table_seek++) {
    double probability;

    if (symbols_count_table[table_seek] == 0) {
      continue;
    }

    probability = ((double) symbols_count_table[table_seek]) / ((double) password_length);
    entropy -= probability * log2(probability);
  }

  /* Do not need the symbol table any more, better to clean that */
  memset(symbols_count_table, 0, OUTPUT_DOMAIN_MAXLENGTH * sizeof(int));

  /* Password strength is function of the entropy, the password length,
   * and the alphabet length.
   * The strength is reduced with increasing year, to take into account
   * the probable increasing computing power of hackers.
   * Unit is completely arbitrary. */
  return entropy * ((double)password_length / (double)(max(year / 5 - 388, 12)))
         * log2((double)alphabet_size) / OVERKILL_PWD_STRENGTH;
}
