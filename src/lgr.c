/**
 *  @file     lgr.c
 *  @brief    lgr.c
 *  @version  v.4
 *  @date     02/15/2017 18:08:02
 *  @author   Anonrate
 *  @copyright
 *    \parblock
 *      GNU General Public License
 *
 *      Copyright (C) 2017 Anonrate
 *
 *      This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 *        the Free Software Foundation, either version 3 of the License, or
 *        (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *        but WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *        GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *        along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *    \endparblock
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <errno.h>
#include  <string.h>
#include  <time.h>
#include  <assert.h>

#define LGR_DEV
#ifdef  LGR_DEV
#ifndef ENABLE_INTERN_WARNING
#define ENABLE_INTERN_WARNING
#endif  /* ENABLE_INTERN_WARNING  */

#ifndef ENABLE_INTERN_INFO
#define ENABLE_INTERN_INFO
#endif  /* ENABLE_INTERN_INFO     */

#ifndef ENABLE_INTERN_DEBUG
#define ENABLE_INTERN_DEBUG
#endif  /* ENABLE_INTERN_DEBUG    */

#ifndef ENABLE_INTERN_TRACE
#define ENABLE_INTERN_TRACE
#endif  /* ENABLE_INTERN_TRACE    */
#endif  /* LGR_DEV                */

#include  "../inc/lgrverblvls.h"

static const char*
getvlvln(enum verblvls verblvl)
{
    return  ((verblvl ==              FATAL)  ? FATAL_STR
           : (verblvl ==              ERROR)  ? ERROR_STR
           : (verblvl ==            WARNING)  ? WARNING_STR
           : (verblvl ==             NOTICE)  ? NOTICE_STR
           : (verblvl ==               INFO)  ? INFO_STR
           : (verblvl ==              DEBUG)  ? DEBUG_STR
           : (verblvl ==              TRACE)  ? TRACE_STR
#ifdef  ENABLE_INTERN_WARNING
           : (verblvl ==     INTERN_WARNING)  ? INTERN_WARNING_STR
#endif  /* ENABLE_INTERN_WARNING  */

#ifdef  ENABLE_INTERN_INFO
           : (verblvl ==        INTERN_INFO)  ? INTERN_INFO_STR
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
           : (verblvl ==       INTERN_DEBUG)  ? INTERN_DEBUG_STR
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_TRACE
           : (verblvl ==       INTERN_TRACE)  ? INTERN_TRACE_STR
#endif  /* ENABLE_INTERN_TRACE    */

#ifdef  LGR_DEV
           : (verblvl == DEV_INTERN_WARNING)  ? DEV_INTERN_WARNING_STR
           : (verblvl ==    DEV_INTERN_INFO)  ? DEV_INTERN_INFO_STR
           : (verblvl ==   DEV_INTERN_DEBUG)  ? DEV_INTERN_DEBUG_STR
           : (verblvl ==   DEV_INTERN_TRACE)  ? DEV_INTERN_TRACE_STR
#endif  /* LGR_DEV                */

           :                                    NVALID_VERB_LVL_STR);
}

#ifdef  LGR_DEV
static char           *vlvln    = DEV_INTERN_TRACE_STR;
static enum verblvls  vlvl      = DEV_INTERN_TRACE;
#else
static char           *vlvln    = WARNING_STR;
static enum verblvls  vlvl      = WARNING;
#endif  /* LGR_DEV */

static int            ltf       = 0;

static enum verblvls  fprio     = ERROR;

static int            errwarn   = 0;

static char           *fnsfxfmt = "%y%m%d%H%M%S";
static char           *fname    = "\0";

static char           *fnout    = "\0";

static FILE           *fout     = 0;

static void
lgrf(enum   verblvls        verblvl,
     const            char  *timestr,
     const            char  *filestr,
     const            char  *funcstr,
     const  unsigned  int   line,
     const            char  *strfmt, ...)
{
    if (!(verblvl > 0
                && verblvl <=
#if   defined LGR_DEV
                              DEV_INTERN_TRACE
#elif defined ENABLE_INTERN_TRACE   /* !defined LGR_DEV               */
                              INTERN_TRACE
#elif defined ENABLE_INTERN_DEBUG   /* !defined ENABLE_INTERN_TRACE   */
                              INTERN_DEBUG
#elif defined ENABLE_INTERN_INFO    /* !defined ENABLE_INTERN_DEBUG   */
                              INTERN_INFO
#elif defined ENABLE_INTERN_WARNING /* !defined ENABLE_INTERN_INFO    */
                              INTERN_WARNING
#else                               /* !defined ENABLE_INTERN_WARNING */
                              TRACE
#endif                              /*
                                     *    LGR_DEV
                                     *  : ENABLE_INTERN_TRACE
                                     *  : ENABLE_INTERN_DEBUG
                                     *  : ENABLE_INTERN_INFO
                                     *  : ENABLE_INTERN_WARNING
                                     *  :
                                     */
         )  ? verblvl
            : NVALID_VERB_LVL) { return; }

    const unsigned char tmpvlvl =
        ((errwarn && verblvl == WARNING) ? ERROR : verblvl);

    if (tmpvlvl > vlvl) { return; }
    FILE *fpstrm  =
        ((errwarn)
         ? ((verblvl <= WARNING)
             ? stderr
             : stdout)
         : ((verblvl <= ERROR)
             ? stderr
             : stdout));


    int doltf = ltf && fout && tmpvlvl <= fprio;

    if (timestr)
    {
        fprintf(fpstrm,   "[%s]  %-18s:  ", timestr,  getvlvln(verblvl));
        if (doltf) {
            fprintf(fout, "[%s]  %-18s:  ", timestr,  getvlvln(verblvl));
        }
    }

    if (filestr || funcstr || line)
    {
        if (filestr)
        {
            fprintf(fpstrm,   "%s:",  filestr);
            if (doltf) {
                fprintf(fout, "%s:",  filestr);
            }
        }

        if (funcstr)
        {
            fprintf(fpstrm,   "%s:",  funcstr);
            if (doltf) {
                fprintf(fout, "%s:",  funcstr);
            }
        }

        if (line)
        {
            fprintf(fpstrm,   "%lu:", line);
            if (doltf) {
                fprintf(fout, "%lu:", line);
            }
        }

        fprintf(fpstrm,             "   ");
        if (doltf) { fprintf(fout,  "   "); }
    }

    va_list ap;
    va_start(ap, strfmt);
    vfprintf(fpstrm,            strfmt, ap);
    if (doltf) { vfprintf(fout, strfmt, ap); }
    va_end(ap);
}

#include  "../inc/lgr.h"

#ifndef NAME_MAX
#define NAME_MAX  0xfe
#endif  /* NAME_MAX */

#if defined ENABLE_INTERN_WARNING \
 || defined ENABLE_INTERN_INFO    \
 || defined ENABLE_INTERN_DEBUG   \
 || defined ENABLE_INTERN_TRACE
#ifndef LGRMSGS_H
#include  "../inc/lgrmsgs.h"
#endif  /* LGRMSGS_H  */
#endif  /*
         *     ENABLE_INTERN_WARNING
         *  || ENABLE_INTERN_INFO
         *  || ENABLE_INTERN_DEBUG
         *  || ENABLE_INTERN_TRACE
         */

#ifdef  LGR_DEV
int
main(int argc, char **argv)
{
    setfilename("lgr");
    return EXIT_SUCCESS;
}
#endif  /* LGR_DEV    */


const char*
getverblvlname(enum verblvls verblvl)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "enum verblvls verblvl = %hhu\n", verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_TRACE
    CALLFN_MSGLS(INTERN_TRACE, "getvlvln()");
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_TRACE,
               "enum verblvls verblvl = verblvl(%hhu)\n",
               verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    const     char *tmpvlvln  = getvlvln(verblvl);

    unsigned  char  tmpvlvl;
#ifdef  ENABLE_INTERN_INFO
    tmpvlvl = INTERN_INFO;
              char  *tmpstr   = ((strcmp(tmpvlvln, NVALID_VERB_LVL_STR))
                              ? " "
                              : (tmpvlvl = WARNING, " not "));
    logltffnlf(tmpvlvl, "%hhu is%sa valid verbose level!\n", verblvl, tmpstr);
#else /* !defined ENABLE_INTERN_INFO  */
    if (strcmp(tmpvlvlv, NVALID_VERB_LVL_STR)) {
        logltffnlf(WARNING, "%hhu is not a valid verbose level!\n", verblvl);
    }
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, tmpvlvln);
#endif  /* ENABLE_INTERN_DEBUG    */
    return tmpvlvln;
}

int
isverblvl(unsigned char lvl)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "unsigned char lvl = %hhu\n", lvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_TRACE
    CALLFN_MSGLS(INTERN_TRACE, "getverblvlname()");
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_TRACE, "enum verblvls verblvl = lvl(%hhu)\n", lvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    if (strcmp(getverblvlname(lvl), NVALID_VERB_LVL_STR))
    {
#ifdef  ENABLE_INTERN_DEBUG
        R_MSGLHHU(INTERN_DEBUG, NVALID_VERB_LVL);
#endif  /* ENABLE_INTERN_DEBUG    */
        return NVALID_VERB_LVL;
    }

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLHHU(INTERN_DEBUG, lvl);
#endif  /* ENABLE_INTERN_DEBUG    */
    return (int)lvl;
}

static void
mallstr(const char *stra, char **pstrb, char *strbn)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "const char *stra   = %s\n", stra);
    logltffnlf(DEV_INTERN_DEBUG, "      char **pstrb = %s\n", *pstrb);
    logltffnlf(DEV_INTERN_DEBUG, "      char *strbn  = %s\n", strbn);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

    size_t tmpstrbsz  = strlen(*pstrb);
    size_t tmpstrasz  = strlen(stra);
    if (tmpstrbsz != tmpstrasz) {
        if (!(*pstrb = malloc(tmpstrasz + 1ul))) {
            fatalf(MALLOC_FAIL_MSGSF, "*pstrb");
        }
    }
}

static char*
setvlvln(enum verblvls verblvl)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "enum verblvls verblvl = %hhu\n", verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_TRACE
    CALLFN_MSGLS(INTERN_TRACE, "getverblvlname()");
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_TRACE,
               "enum verblvls verblvl = verblvl(%hhu)\n",
               verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    const char *tmpvlvln = getverblvlname(verblvl);
    if (!strcmp(vlvln, tmpvlvln))
    {
#ifdef  ENABLE_INTERN_DEBUG
        R_MSGLS(INTERN_DEBUG, vlvln);
#endif  /* ENABLE_INTERN_DEBUG    */
        return vlvln;
    }

    if (strcmp(tmpvlvln, NVALID_VERB_LVL_STR))
    {
#ifdef  ENABLE_INTERN_TRACE
        CALLFN_MSGLS(INTERN_TRACE, "mallstr()");
#ifdef  LGR_DEV
        MALLSTR_DEVMSGSS(tmpvlvln, vlvln);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
        mallstr(tmpvlvln, &vlvln, "vlvln");

#ifdef  ENABLE_INTERN_INFO
        SET_MSGLSS(INTERN_INFO, vlvln, tmpvlvln);
#endif  /* ENABLE_INTERN_INFO     */
        strcpy(vlvln, tmpvlvln);

#ifdef  ENABLE_INTERN_DEBUG
        R_MSGLS(INTERN_DEBUG, vlvln);
#endif  /* ENABLE_INTERN_DEBUG    */
        return vlvln;
    }

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, vlvln);
#endif  /* ENABLE_INTERN_DEBUG    */
    return vlvln;
}

static unsigned char
setvlvl(unsigned char verblvl)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "unsigned char verblvl = %hhu\n", verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    SET_MSGLHHUHHU(INTERN_INFO, vlvl, verblvl);
#endif  /* ENABLE_INTERN_INFO     */
    vlvl = verblvl;

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLHHU(INTERN_DEBUG, verblvl);
#endif  /* ENABLE_INTERN_DEBUG    */
    return (verblvl);
}

unsigned char
setverblvl(enum verblvls verblvl)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "enum verblvls verblvl = %hhu\n", verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

    if (vlvl == verblvl)
    {
#ifdef  ENABLE_INTERN_DEBUG
        R_MSGLHHU(INTERN_DEBUG, vlvl);
#endif  /* ENABLE_INTERN_DEBUG    */
        return vlvl;
    }

#ifdef  ENABLE_INTERN_TRACE
    CALLFN_MSGLS(INTERN_TRACE, "isverblvl()");
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_TRACE,
               "unsigned char verblvl = verblvl(%hhu)\n",
               verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    if (isverblvl(verblvl))
    {
#ifdef  ENABLE_INTERN_TRACE
        CALLFN_MSGLS(INTERN_TRACE, "setvlvl()");
#ifdef  LGR_DEV
        logltffnlf(DEV_INTERN_TRACE,
                   "unsigned char verblvl = verblvl(%hhu)\n",
                   verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
        setvlvl(verblvl);

#ifdef  ENABLE_INTERN_TRACE
        CALLFN_MSGLS(INTERN_TRACE, "setvlvln()");
#ifdef  LGR_DEV
        logltffnlf(DEV_INTERN_TRACE,
                   "unsigned char verblvl = verblvl(%hhu)\n",
                   verblvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
        setvlvln(verblvl);

#ifdef  ENABLE_INTERN_TRACE
        CALLFN_MSGLS(INTERN_TRACE, "getverblvlname()");
#ifdef  LGR_DEV
        logltffnlf(DEV_INTERN_TRACE,
                   "enum verblvls verblvl = vlvl(%hhu)\n",
                   vlvl);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
        int ti = strcmp(vlvln, getverblvlname(vlvl));
        if (ti) { fatalf("Validation failed..  strcmp returned %d.", ti); }

#ifdef  ENABLE_INTERN_DEBUG
        R_MSGLD(INTERN_DEBUG, vlvl);
#endif  /* ENABLE_INTERN_DEBUG    */
        return vlvl;
    }
#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLHHU(INTERN_DEBUG, vlvl);
#endif  /* ENABLE_INTERN_DEBUG    */
    return vlvl;
}

enum verblvls
getverblvl(void)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    GET_MSGLS(INTERN_INFO, "vlvl");
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, vlvl);
#endif  /* ENABLE_INTERN_DEBUG    */
    return vlvl;
}

enum verblvls
getfileprio(void)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    GET_MSGLS(INTERN_INFO, "fprio");
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, fprio);
#endif  /* ENABLE_INTERN_DEBUG    */
    return fprio;
}

int
geterrwarn(void)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    GET_MSGLS(INTERN_INFO, "errwarn");
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLD(INTERN_DEBUG, errwarn);
#endif  /* ENABLE_INTERN_DEBUG    */
    return errwarn;
}

enum verblvls
setfileprio(enum verblvls fileprio)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "enum verblvls fileprio = %hhu\n", fileprio);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_TRACE
    CALLFN_MSGLS(INTERN_TRACE, "isverblvl()");
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_TRACE,
               "unsigned char lvl = fileprio(%s)\n",
               fileprio);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    unsigned char tmpvlvl = isverblvl(fileprio);

#ifdef  ENABLE_INTERN_INFO
    SET_MSGLHHUHHU(INTERN_INFO, fprio, fileprio);
#endif  /* ENABLE_INTERN_INFO     */
    fprio = fileprio;

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, fprio);
#endif  /* ENABLE_INTERN_DEBUG    */
    return fprio;
}

int
seterrwarn(int treatwarnerr)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "int treatwarnerr = %d\n", treatwarnerr);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    SET_MSGLDD(INTERN_INFO, errwarn, treatwarnerr);
#endif  /* ENABLE_INTERN_INFO     */
    errwarn = treatwarnerr;

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLD(INTERN_DEBUG, errwarn);
#endif  /* ENABLE_INTERN_DEBUG    */
    return errwarn;
}

static char*
setfout(void)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#endif  /* ENABLE_INTERN_DEBUG    */

    if (!fname)
    {
    }

    time_t t        = time(0);

    struct tm *ti   = localtime(&t);
    if (!ti) { fatalstr(strerror(errno)); }

#ifdef  ENABLE_INTERN_DEBUG
    logltffnlf(INTERN_DEBUG, "malloc(%lu)\n", NAME_MAX);
#endif  /* ENABLE_INTERN_DEBUG    */
    char *tmpfno    = malloc(NAME_MAX);
    if (!tmpfno) { fatalf(MALLOC_FAIL_MSGSF, "tmpfno"); }

    size_t tmpfnosz = strftime(tmpfno, NAME_MAX, fnsfxfmt, ti);
    if (!tmpfnosz) { fatalf(STRZ_MSGSF, "tmpfno"); }

    tmpfnosz        = sprintf(tmpfno, "%s-%s", tmpfno, fname);
    if (!tmpfnosz) { fatalf(STRZ_MSGSF, "tmpfno"); }

    if (!realloc(tmpfno, tmpfnosz + 1)) {
        REALLOC_FAIL_MSGULUL(tmpfno, tmpfnosz);
    }

#ifdef  ENABLE_INTERN_TRACE
        CALLFN_MSGLS(INTERN_TRACE, "mallstr()");
#ifdef  LGR_DEV
        MALLSTR_DEVMSGSS(tmpfno, fnout);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    mallstr(tmpfno, &fnout, "fnout");

#ifdef  ENABLE_INTERN_INFO
    SET_MSGLSS(INTERN_INFO, fnout, tmpfno);
#endif  /* ENABLE_INTERN_INFO     */
    fnout = tmpfno;

#ifdef  ENABLE_INTERN_INFO
    logltffnlf(INTERN_INFO, "Updating fout(%s)...\n", fnout);
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
    logltffnlf(INTERN_DEBUG,
               "Opening file fout(%s) in append mode...\n",
               fnout);
#endif  /* ENABLE_INTERN_DEBUG    */
    fout = fopen(fnout, "a");

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, fnout);
#endif  /* ENABLE_INTERN_DEBUG    */
    return fnout;
}

char*
setfilename(char *filename)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "char *filename = %s\n", filename);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

    if (!filename)
    {
#ifdef  ENABLE_INTERN_DEBUG
        R_MSGLS(INTERN_DEBUG, fname);
#endif  /* ENABLE_INTERN_DEBUG    */
        return fname;
    }

#ifdef  ENABLE_INTERN_TRACE
    CALLFN_MSGLS(INTERN_TRACE, "mallstr()");
#ifdef  LGR_DEV
    MALLSTR_DEVMSGSS(filename, fname);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_TRACE    */
    mallstr(filename, &fname, "fname");
#ifdef  ENABLE_INTERN_INFO
    SET_MSGLSS(INTERN_INFO, fname, filename);
#endif  /* ENABLE_INTERN_INFO     */
    fname = filename;
    setfout();

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLS(INTERN_DEBUG, fname);
#endif  /* ENABLE_INTERN_DEBUG    */
    return fname;
}

int
setlogtofile(int logtofile)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#ifdef  LGR_DEV
    logltffnlf(DEV_INTERN_DEBUG, "int logtofile = %d\n", logtofile);
#endif  /* LGR_DEV                */
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    SET_MSGLDD(INTERN_INFO, ltf, logtofile);
#endif  /* ENABLE_INTERN_INFO     */
    ltf = logtofile;

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLD(INTERN_DEBUG, ltf);
#endif  /* ENABLE_INTERN_DEBUG    */
    return ltf;
}

int
getlogtofile(void)
{
#ifdef  ENABLE_INTERN_DEBUG
    INFUNC_MSGL(INTERN_DEBUG);
#endif  /* ENABLE_INTERN_DEBUG    */

#ifdef  ENABLE_INTERN_INFO
    GET_MSGLS(INTERN_INFO, "ltf");
#endif  /* ENABLE_INTERN_INFO     */

#ifdef  ENABLE_INTERN_DEBUG
    R_MSGLD(INTERN_DEBUG, ltf);
#endif  /* ENABLE_INTERN_DEBUG    */
    return ltf;
}
