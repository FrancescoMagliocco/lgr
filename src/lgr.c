/**
 *  @file     lgr.c
 *  @brief    lgr.c
 *  @version  v.5
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
#undef  LGR_DEV

#include  "../inc/lgr.h"
#include  "../inc/lgrmsgs.h"

static const char*
getvlvln(enum verblvls verblvl)
{
    return  ((verblvl ==          FATAL)  ? FATAL_STR
           : (verblvl ==          ERROR)  ? ERROR_STR
           : (verblvl ==        WARNING)  ? WARNING_STR
           : (verblvl ==         NOTICE)  ? NOTICE_STR
           : (verblvl ==           INFO)  ? INFO_STR
           : (verblvl ==          DEBUG)  ? DEBUG_STR
           : (verblvl ==          TRACE)  ? TRACE_STR
           : (verblvl == INTERN_WARNING)  ? INTERN_WARNING_STR
           : (verblvl ==    INTERN_INFO)  ? INTERN_INFO_STR
           : (verblvl ==   INTERN_DEBUG)  ? INTERN_DEBUG_STR
           : (verblvl ==   INTERN_TRACE)  ? INTERN_TRACE_STR
           :                                NVALID_VERB_LVL_STR);
}


static enum verblvls  fprio     = ERROR;
static char           *fnsfxfmt = "%y%m%d%H%M%S";
static char           *fname    = "\0";

static char           *fnout    = "\0";

static FILE           *fout     = 0;

int usecolr           = 1;
int enableinternmsgs  = 0;
int erronwarn         = 0;
int logtofile         = 0;

struct fmtfgbgc_s defattrb      = { RS_ALL, FG_DEF, BG_DEF };
struct fmtfgbgc_s fatalattrb    = { RS_ALL, FG_RED, BG_DEF };
struct fmtfgbgc_s errorattrb    = { RS_ALL, FG_LIGHT_RED, BG_DEF };
struct fmtfgbgc_s warrningattrb = { RS_ALL, FG_YELLOW, BG_DEF };
struct fmtfgbgc_s noticeattrb   = { RS_ALL, FG_LIGHT_BLUE, BG_DEF };
struct fmtfgbgc_s infoattrb     = { RS_ALL, FG_LIGHT_YELLOW, BG_DEF };
struct fmtfgbgc_s debugattrb    = { RS_ALL, FG_MAGENTA, BG_DEF };
struct fmtfgbgc_s traceattrb    = { RS_ALL, FG_LIGHT_MAGENTA, BG_DEF };

struct fmtfgbgc_s timestrattrb  = { RS_ALL, FG_LIGHT_GREEN, BG_DEF };
struct fmtfgbgc_s filestrattrb  = { RS_ALL, FG_YELLOW, BG_DEF };
struct fmtfgbgc_s funcstrattrb  = { RS_ALL, FG_GREEN, BG_DEF };
struct fmtfgbgc_s lineattrb     = { RS_ALL, FG_RED, BG_DEF };

struct vnfo_s {
    enum  verblvls  vlvl;
    const char      *((*vlvln)());
};

static const char*
vlvln(struct vnfo_s *pvnfo)
{
    return getvlvln(pvnfo->vlvl);
}

static struct vnfo_s vnfo = {
    .vlvl  =
#ifdef  LGR_DEV
             INTERN_TRACE,
#else
             WARNING,
#endif  /* LGR_DEV */
    .vlvln = vlvln };

static void
printtimestr(       FILE        *strm,
             const  char        *timestr,
             struct fmtfgbgc_s  vattrb,
             struct vnfo_s      verbnfo,
                    int         doltf)
{
    if (usecolr) {
        fprintf(strm,
                "\e[%u;%u;%um[\e[%u;%u;%um%s\e[%u;%u;%um]\e[%u;%u;%um "
                    "\e[%u;%u;%um%-18s\e[%u;%u;%um:\e[%u;%u;%um  ",
                debugattrb.fmt, debugattrb.fgc, debugattrb.bgc,
                timestrattrb.fmt, timestrattrb.fgc, timestrattrb.bgc,
                timestr,
                debugattrb.fmt, debugattrb.fgc, debugattrb.bgc,
                defattrb.fmt, defattrb.fgc, defattrb.bgc,
                vattrb.fmt, vattrb.fgc, vattrb.bgc,
                verbnfo.vlvln(verbnfo),
                debugattrb.fmt, debugattrb.fgc, debugattrb.bgc,
                defattrb.fmt, defattrb.fgc, defattrb.bgc);
    } else {
        fprintf(strm, "[%s]  %-18s:  ", timestr, verbnfo.vlvln(verbnfo));
    }

    if (doltf) {
        fprintf(fout, "[%s]  %-18s:  ", timestr, verbnfo.vlvln(verbnfo));
    }
}

static void
printfilefnstr(       FILE        *strm,
               struct fmtfgbgc_s  fileorfnstrattrb,
               const  char        *fileorfnstr,
                      int         doltf)
{
    if (usecolr) {
        fprintf(strm,
                "\e[%u;%u;%um%s\e[%u;%u;%um:\e[%u;%u;%um",
                fileorfnstrattrb.fmt,
                    fileorfnstrattrb.fgc,
                    fileorfnstrattrb.bgc,
                fileorfnstr,
                debugattrb.fmt, debugattrb.fgc, debugattrb.bgc,
                defattrb.fmt, defattrb.fgc, defattrb.bgc);
    } else     { fprintf(strm, "%s:", fileorfnstr); }
    if (doltf) { fprintf(fout, "%s:", fileorfnstr); }
}

static void
printline(          FILE  *strm,
          unsigned  int   line,
                    int   doltf)
{
    if (usecolr) {
        fprintf(strm,
                "\e[%u;%u;%um%u\e[%u;%u;%um:\e[%u;%u;%um",
                lineattrb.fmt, lineattrb.fgc, lineattrb.bgc,
                line,
                debugattrb.fmt, debugattrb.fgc, debugattrb.bgc,
                defattrb.fmt, defattrb.fgc, defattrb.bgc);
    } else     { fprintf(strm, "%u:", line); }
    if (doltf) { fprintf(fout, "%u:", line); }
}

static struct vnfo_s
getvnfo(enum verblvls verblvl)
{
    struct vnfo_s tvnfo = { .vlvl   = verblvl,
                            .vlvln  = vlvln };
    return tvnfo;
}

static struct fmtfgbgc_s
getvattrb(enum verblvls verblvl)
{
    return ((verblvl ==   FATAL) ? fatalattrb
          : (verblvl ==   ERROR) ? errorattrb
          : (verblvl == WARNING) ? warrningattrb
          : (verblvl ==  NOTICE) ? noticeattrb
          : (verblvl ==    INFO) ? infoattrb
          : (verblvl ==   DEBUG) ? debugattrb
          :                        traceattrb);
}

void
lgrf(enum   verblvls        verblvl,
     const            char  *timestr,
     const            char  *filestr,
     const            char  *funcstr,
     const  unsigned  int   line,
     const            char  *strfmt, ...)
{
    if (!((verblvl > 0)
                && (verblvl <= INTERN_TRACE))
            ? verblvl
            : NVALID_VERB_LVL) { return; }

    const unsigned char tmpvlvl =
        ((erronwarn && (verblvl == WARNING)) ? ERROR : verblvl);

    if ((tmpvlvl > vnfo.vlvl)
            && (enableinternmsgs
                && (tmpvlvl < TRACE))) { return; }

    FILE *strm  =
        ((erronwarn)
         ? ((verblvl <= WARNING)
             ? stderr
             : stdout)
         : ((verblvl <= ERROR)
             ? stderr
             : stdout));

    int doltf =
        (logtofile
         && fout
         && ((tmpvlvl <= fprio)
             || enableinternmsgs));

    struct fmtfgbgc_s vattrb  = getvattrb(verblvl);
    struct vnfo_s     tvnfo   = getvnfo(verblvl);

    if (timestr) { printtimestr(strm, timestr, vattrb, tvnfo, doltf); }
    if (filestr || funcstr || line)
    {
        if (filestr) { printfilefnstr(strm, filestrattrb, filestr, doltf); }
        if (funcstr) { printfilefnstr(strm, funcstrattrb, funcstr, doltf); }
        if (line)    { printline(strm, line, doltf); }

        fprintf(strm,               "   ");
        if (doltf) { fprintf(fout,  "   "); }
    }

    if (usecolr) {
        fprintf(strm, "\e[%u;%u;%um", vattrb.fmt, vattrb.fgc, vattrb.bgc);
    }

    va_list ap;
    va_start(ap, strfmt);
    vfprintf(strm,              strfmt, ap);
    if (doltf) { vfprintf(fout, strfmt, ap); }
    va_end(ap);

    if (usecolr) {
        fprintf(strm,
                "\e[%u;%u;%um",
                defattrb.fmt, defattrb.fgc, defattrb.bgc);
    }
}

#ifndef NAME_MAX
#define NAME_MAX  0xfe
#endif  /* NAME_MAX */

#ifdef  LGR_DEV
int
main(int argc, char **argv)
{
    setverblvl(TRACE);
    setfileprio(TRACE);
    setlogtofile(1);
    setfilename("lgr");
    logltffnlf(INFO, "%s\n", "Doing this shit.");
    return EXIT_SUCCESS;
}
#endif  /* LGR_DEV    */

const char*
getverblvlname(enum verblvls verblvl)
{
    INFUNC_MSGL(INTERN_DEBUG);
    return R_MSGLS(INTERN_DEBUG, getvlvln(verblvl));
}

int
isverblvl(unsigned char lvl)
{
    INFUNC_MSGL(INTERN_DEBUG);
    CALLFN_MSGLS(INTERN_TRACE, "getverblvlname()");
    return R_MSGLD(INTERN_DEBUG,
                   (strcmp(getverblvlname(lvl), NVALID_VERB_LVL_STR)
                    ? (int)lvl
                    : NVALID_VERB_LVL));
}

static void
mallstr(const char *stra, char **pstrb)
{
    if (enableinternmsgs) { INFUNC_MSGL(INTERN_DEBUG); }
    size_t tmpstrbsz  = strlen(*pstrb);
    size_t tmpstrasz  = strlen(stra);
    if (tmpstrbsz != tmpstrasz) { *pstrb = malloc(tmpstrasz + 1lu); }
}

unsigned char
setverblvl(enum verblvls verblvl)
{
    INFUNC_MSGL(INTERN_DEBUG);
    if (vnfo.vlvl == verblvl) { return R_MSGLHHU(INTERN_DEBUG, vnfo.vlvl); }

    CALLFN_MSGLS(INTERN_TRACE, "isverblvl()");
    if (isverblvl(verblvl)) { vnfo.vlvl = verblvl; }

    return R_MSGLHHU(INTERN_DEBUG, vnfo.vlvl);
}

enum verblvls
getverblvl(void)
{
    INFUNC_MSGL(INTERN_DEBUG);
    return R_MSGLU(INTERN_DEBUG, vnfo.vlvl);
}

enum verblvls
getfileprio(void)
{
    INFUNC_MSGL(INTERN_DEBUG);
    return R_MSGLU(INTERN_DEBUG, fprio);
}

enum verblvls
setfileprio(enum verblvls fileprio)
{
    INFUNC_MSGL(INTERN_DEBUG);
    CALLFN_MSGLS(INTERN_TRACE, "isverblvl()");

    if (!isverblvl(fileprio)) {
        return R_MSGLHHU(INTERN_DEBUG, (unsigned char)0);
    }

    return R_MSGLHHU(INTERN_DEBUG, (fprio = fileprio));
}

static char*
setfout(void)
{
    if (enableinternmsgs) { INFUNC_MSGL(INTERN_DEBUG); }
    time_t    t       = time(0);
    struct tm *ti     = localtime(&t);
    char      *tmpfno = malloc(NAME_MAX);

    strftime(tmpfno, NAME_MAX, fnsfxfmt, ti);

    size_t tmpfnosz = sprintf(tmpfno, "%s-%s", tmpfno, fname);

    realloc(tmpfno, tmpfnosz + 1lu);
    if (enableinternmsgs) { CALLFN_MSGLS(INTERN_TRACE, "mallstr()"); }
    mallstr(tmpfno, &fnout);

    fnout = tmpfno;
    fout  = fopen(fnout, "a");

    if (enableinternmsgs) { return R_MSGLS(INTERN_DEBUG, fnout); }
    return fnout;
}

char*
setfilename(char *filename)
{
    INFUNC_MSGL(INTERN_DEBUG);
    if (!filename || !logtofile) { return R_MSGLS(INTERN_DEBUG, fname); }

    CALLFN_MSGLS(INTERN_TRACE, "mallstr()");
    mallstr(filename, &fname);

    fname = filename;

    CALLFN_MSGLS(INTERN_TRACE, "setfout()");
    setfout();

    return R_MSGLS(INTERN_DEBUG, fname);
}

char*
setfilenamesuffixfmt(const char *suffixfmt)
{
    INFUNC_MSGL(INTERN_DEBUG);
    time_t    t       = time(0);
    struct tm *ti     = localtime(&t);
    char      *tmpfno = malloc(NAME_MAX);

    size_t tmpfnosz = strftime(tmpfno, NAME_MAX, fnsfxfmt, ti);
    if (!tmpfnosz) { return R_MSGLS(INTERN_DEBUG, fnsfxfmt); }

    CALLFN_MSGLS(INTERN_TRACE, "mallstr()");
    mallstr(suffixfmt, &fnsfxfmt);
    strcpy(fnsfxfmt, suffixfmt);

    return R_MSGLS(INTERN_DEBUG, fnsfxfmt);
}

char*
getfilenamesuffixfmt(void)
{
    INFUNC_MSGL(INTERN_DEBUG);
    return R_MSGLS(INTERN_DEBUG, fnsfxfmt);
}

char*
getfilenameout(void)
{
    INFUNC_MSGL(INTERN_DEBUG);
    return R_MSGLS(INTERN_DEBUG, fnout);
}

int
closeout(void)
{
    INFUNC_MSGL(INTERN_DEBUG);
    CALLFN_MSGLS(INTERN_TRACE, "fclose()");
    return R_MSGLD(INTERN_DEBUG, !fclose(fout));
}

int
dellog(void)
{
    INFUNC_MSGL(INTERN_DEBUG);
    if (!fout) { return R_MSGLD(INTERN_DEBUG, 1); }

    CALLFN_MSGLS(INTERN_TRACE, "closeout()");
    return R_MSGLD(INTERN_DEBUG, (closeout() ? !remove(fnout) : 0));
}
