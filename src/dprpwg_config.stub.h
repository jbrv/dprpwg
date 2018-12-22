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

#ifndef DPRPWG_CONFIG
#define DPRPWG_CONFIG

/*
 * Pseudo-Random generator configuration.
 *
 * ***** WARNING! ***** WARNING! ***** WARNING! *****
 * *  IF THAT CHANGES, PASSWORD GENERATION CHANGES  *
 * **************************************************
 *
 * Write some numbers below to replace all the "<YOUR_NUMBER>"
 * occurences. Then, move this file to "dprpwg_config.h" AND DO NOT
 * MODIFY IT. Leave the "U"s for "unsigned".
 * I don't really know, but maybe prime numbers are better... Feel free
 * to do the mathematical analysis if you want.
 *
 * I do not want to commit the number I personally use. It's probably not
 * so clever, because I don't know if the "hash" algorithm is good. So,
 * I ended up with this solution. Maybe I will commit them in a branch
 * that I will not push publicly.
 */

#define PW_MUL      (<YOUR_NUMBER>U)
#define PW_SEEK_MUL (<YOUR_NUMBER>U)
#define PW_INV_MUL  (<YOUR_NUMBER>U)

#define DOM_MUL       (<YOUR_NUMBER>U)
#define DOM_SEEK_MUL  (<YOUR_NUMBER>U)
#define DOM_INV_MUL   (<YOUR_NUMBER>U)

#define YR_MUL      (<YOUR_NUMBER>U)
#define YR_SEEK_MUL (<YOUR_NUMBER>U)
#define YR_INV_MUL  (<YOUR_NUMBER>U)

#endif /* DPRPWG_CONFIG */
