#pragma once 

#include <stdint.h>
#include <ctype.h>



typedef int64_t Timestamp;
typedef int64_t TimestampTz;
typedef int64_t TimeOffset;
typedef int32_t fsec_t;			/* fractional seconds (in microseconds) */

typedef struct
{
	TimeOffset	time;			/* all time units other than days, months and
								 * years */
	int32_t		day;			/* days, after time for alignment */
	int32_t		month;			/* months and years, after time for alignment */
} Interval;


/* Limits on the "precision" option (typmod) for these data types */
#define MAX_TIMESTAMP_PRECISION 6
#define MAX_INTERVAL_PRECISION 6

/*
 *	Round off to MAX_TIMESTAMP_PRECISION decimal places.
 *	Note: this is also used for rounding off intervals.
 */
#define TS_PREC_INV 1000000.0
#define TSROUND(j) (rint(((double) (j)) * TS_PREC_INV) / TS_PREC_INV)


/*
 * Assorted constants for datetime-related calculations
 */

#define DAYS_PER_YEAR	365.25	/* assumes leap year every four years */
#define MONTHS_PER_YEAR 12
/*
 *	DAYS_PER_MONTH is very imprecise.  The more accurate value is
 *	365.2425/12 = 30.436875, or '30 days 10:29:06'.  Right now we only
 *	return an integral number of days, but someday perhaps we should
 *	also return a 'time' value to be used as well.  ISO 8601 suggests
 *	30 days.
 */
#define DAYS_PER_MONTH	30		/* assumes exactly 30 days per month */
#define HOURS_PER_DAY	24		/* assume no daylight savings time changes */

/*
 *	This doesn't adjust for uneven daylight savings time intervals or leap
 *	seconds, and it crudely estimates leap years.  A more accurate value
 *	for days per years is 365.2422.
 */
#define SECS_PER_YEAR	(36525 * 864)	/* avoid floating-point computation */
#define SECS_PER_DAY	86400
#define SECS_PER_HOUR	3600
#define SECS_PER_MINUTE 60
#define MINS_PER_HOUR	60

#define INT64CONST(x)  (x##L)
#define USECS_PER_DAY	INT64CONST(86400000000)
#define USECS_PER_HOUR	INT64CONST(3600000000)
#define USECS_PER_MINUTE INT64CONST(60000000)
#define USECS_PER_SEC	INT64CONST(1000000)

/*
 * We allow numeric timezone offsets up to 15:59:59 either way from Greenwich.
 * Currently, the record holders for wackiest offsets in actual use are zones
 * Asia/Manila, at -15:56:00 until 1844, and America/Metlakatla, at +15:13:42
 * until 1867.  If we were to reject such values we would fail to dump and
 * restore old timestamptz values with these zone settings.
 */
#define MAX_TZDISP_HOUR		15	/* maximum allowed hour part */
#define TZDISP_LIMIT		((MAX_TZDISP_HOUR + 1) * SECS_PER_HOUR)

/*
 * DT_NOBEGIN represents timestamp -infinity; DT_NOEND represents +infinity
 */
#define DT_NOBEGIN		PG_INT64_MIN
#define DT_NOEND		PG_INT64_MAX

#define TIMESTAMP_NOBEGIN(j)	\
	do {(j) = DT_NOBEGIN;} while (0)

#define TIMESTAMP_IS_NOBEGIN(j) ((j) == DT_NOBEGIN)

#define TIMESTAMP_NOEND(j)		\
	do {(j) = DT_NOEND;} while (0)

#define TIMESTAMP_IS_NOEND(j)	((j) == DT_NOEND)

#define TIMESTAMP_NOT_FINITE(j) (TIMESTAMP_IS_NOBEGIN(j) || TIMESTAMP_IS_NOEND(j))


/*
 * Julian date support.
 *
 * date2j() and j2date() nominally handle the Julian date range 0..INT_MAX,
 * or 4714-11-24 BC to 5874898-06-03 AD.  In practice, date2j() will work and
 * give correct negative Julian dates for dates before 4714-11-24 BC as well.
 * We rely on it to do so back to 4714-11-01 BC.  Allowing at least one day's
 * slop is necessary so that timestamp rotation doesn't produce dates that
 * would be rejected on input.  For example, '4714-11-24 00:00 GMT BC' is a
 * legal timestamptz value, but in zones east of Greenwich it would print as
 * sometime in the afternoon of 4714-11-23 BC; if we couldn't process such a
 * date we'd have a dump/reload failure.  So the idea is for IS_VALID_JULIAN
 * to accept a slightly wider range of dates than we really support, and
 * then we apply the exact checks in IS_VALID_DATE or IS_VALID_TIMESTAMP,
 * after timezone rotation if any.  To save a few cycles, we can make
 * IS_VALID_JULIAN check only to the month boundary, since its exact cutoffs
 * are not very critical in this scheme.
 *
 * It is correct that JULIAN_MINYEAR is -4713, not -4714; it is defined to
 * allow easy comparison to tm_year values, in which we follow the convention
 * that tm_year <= 0 represents abs(tm_year)+1 BC.
 */

#define JULIAN_MINYEAR (-4713)
#define JULIAN_MINMONTH (11)
#define JULIAN_MINDAY (24)
#define JULIAN_MAXYEAR (5874898)
#define JULIAN_MAXMONTH (6)
#define JULIAN_MAXDAY (3)

#define IS_VALID_JULIAN(y,m,d) \
	(((y) > JULIAN_MINYEAR || \
	  ((y) == JULIAN_MINYEAR && ((m) >= JULIAN_MINMONTH))) && \
	 ((y) < JULIAN_MAXYEAR || \
	  ((y) == JULIAN_MAXYEAR && ((m) < JULIAN_MAXMONTH))))

/* Julian-date equivalents of Day 0 in Unix and Postgres reckoning */
#define UNIX_EPOCH_JDATE		2440588 /* == date2j(1970, 1, 1) */
#define POSTGRES_EPOCH_JDATE	2451545 /* == date2j(2000, 1, 1) */

/*
 * Range limits for dates and timestamps.
 *
 * We have traditionally allowed Julian day zero as a valid datetime value,
 * so that is the lower bound for both dates and timestamps.
 *
 * The upper limit for dates is 5874897-12-31, which is a bit less than what
 * the Julian-date code can allow.  For timestamps, the upper limit is
 * 294276-12-31.  The int64 overflow limit would be a few days later; again,
 * leaving some slop avoids worries about corner-case overflow, and provides
 * a simpler user-visible definition.
 */

/* First allowed date, and first disallowed date, in Julian-date form */
#define DATETIME_MIN_JULIAN (0)
#define DATE_END_JULIAN (2147483494)	/* == date2j(JULIAN_MAXYEAR, 1, 1) */
#define TIMESTAMP_END_JULIAN (109203528)	/* == date2j(294277, 1, 1) */

/* Timestamp limits */
#define MIN_TIMESTAMP	INT64CONST(-211813488000000000)
/* == (DATETIME_MIN_JULIAN - POSTGRES_EPOCH_JDATE) * USECS_PER_DAY */
#define END_TIMESTAMP	INT64CONST(9223371331200000000)
/* == (TIMESTAMP_END_JULIAN - POSTGRES_EPOCH_JDATE) * USECS_PER_DAY */

/* Range-check a date (given in Postgres, not Julian, numbering) */
#define IS_VALID_DATE(d) \
	((DATETIME_MIN_JULIAN - POSTGRES_EPOCH_JDATE) <= (d) && \
	 (d) < (DATE_END_JULIAN - POSTGRES_EPOCH_JDATE))

/* Range-check a timestamp */
#define IS_VALID_TIMESTAMP(t)  (MIN_TIMESTAMP <= (t) && (t) < END_TIMESTAMP)


#define DAGO			"ago"
#define DCURRENT		"current"
#define EPOCH			"epoch"
#define INVALID			"invalid"
#define EARLY			"-infinity"
#define LATE			"infinity"
#define NOW				"now"
#define TODAY			"today"
#define TOMORROW		"tomorrow"
#define YESTERDAY		"yesterday"
#define ZULU			"zulu"

#define DMICROSEC		"usecond"
#define DMILLISEC		"msecond"
#define DSECOND			"second"
#define DMINUTE			"minute"
#define DHOUR			"hour"
#define DDAY			"day"
#define DWEEK			"week"
#define DMONTH			"month"
#define DQUARTER		"quarter"
#define DYEAR			"year"
#define DDECADE			"decade"
#define DCENTURY		"century"
#define DMILLENNIUM		"millennium"
#define DA_D			"ad"
#define DB_C			"bc"
#define DTIMEZONE		"timezone"

/*
 * Fundamental time field definitions for parsing.
 *
 *	Meridian:  am, pm, or 24-hour style.
 *	Millennium: ad, bc
 */

#define AM		0
#define PM		1
#define HR24	2

#define AD		0
#define BC		1

/*
 * Field types for time decoding.
 *
 * Can't have more of these than there are bits in an unsigned int
 * since these are turned into bit masks during parsing and decoding.
 *
 * Furthermore, the values for YEAR, MONTH, DAY, HOUR, MINUTE, SECOND
 * must be in the range 0..14 so that the associated bitmasks can fit
 * into the left half of an INTERVAL's typmod value.  Since those bits
 * are stored in typmods, you can't change them without initdb!
 */

#define RESERV	0
#define MONTH	1
#define YEAR	2
#define DAY		3
#define JULIAN	4
#define TZ		5				/* fixed-offset timezone abbreviation */
#define DTZ		6				/* fixed-offset timezone abbrev, DST */
#define DYNTZ	7				/* dynamic timezone abbreviation */
#define IGNORE_DTF	8
#define AMPM	9
#define HOUR	10
#define MINUTE	11
#define SECOND	12
#define MILLISECOND 13
#define MICROSECOND 14
#define DOY		15
#define DOW		16
#define UNITS	17
#define ADBC	18
/* these are only for relative dates */
#define AGO		19
#define ABS_BEFORE		20
#define ABS_AFTER		21
/* generic fields to help with parsing */
#define ISODATE 22
#define ISOTIME 23
/* these are only for parsing intervals */
#define WEEK		24
#define DECADE		25
#define CENTURY		26
#define MILLENNIUM	27
/* hack for parsing two-word timezone specs "MET DST" etc */
#define DTZMOD	28				/* "DST" as a separate word */
/* reserved for unrecognized string values */
#define UNKNOWN_FIELD	31

/*
 * Token field definitions for time parsing and decoding.
 *
 * Some field type codes (see above) use these as the "value" in datetktbl[].
 * These are also used for bit masks in DecodeDateTime and friends
 *	so actually restrict them to within [0,31] for now.
 * - thomas 97/06/19
 * Not all of these fields are used for masks in DecodeDateTime
 *	so allow some larger than 31. - thomas 1997-11-17
 *
 * Caution: there are undocumented assumptions in the code that most of these
 * values are not equal to IGNORE_DTF nor RESERV.  Be very careful when
 * renumbering values in either of these apparently-independent lists :-(
 */

#define DTK_NUMBER		0
#define DTK_STRING		1

#define DTK_DATE		2
#define DTK_TIME		3
#define DTK_TZ			4
#define DTK_AGO			5

#define DTK_SPECIAL		6
#define DTK_EARLY		9
#define DTK_LATE		10
#define DTK_EPOCH		11
#define DTK_NOW			12
#define DTK_YESTERDAY	13
#define DTK_TODAY		14
#define DTK_TOMORROW	15
#define DTK_ZULU		16

#define DTK_DELTA		17
#define DTK_SECOND		18
#define DTK_MINUTE		19
#define DTK_HOUR		20
#define DTK_DAY			21
#define DTK_WEEK		22
#define DTK_MONTH		23
#define DTK_QUARTER		24
#define DTK_YEAR		25
#define DTK_DECADE		26
#define DTK_CENTURY		27
#define DTK_MILLENNIUM	28
#define DTK_MILLISEC	29
#define DTK_MICROSEC	30
#define DTK_JULIAN		31

#define DTK_DOW			32
#define DTK_DOY			33
#define DTK_TZ_HOUR		34
#define DTK_TZ_MINUTE	35
#define DTK_ISOYEAR		36
#define DTK_ISODOW		37


/*
 * Bit mask definitions for time parsing.
 */

#define DTK_M(t)		(0x01 << (t))

/* Convenience: a second, plus any fractional component */
#define DTK_ALL_SECS_M	(DTK_M(SECOND) | DTK_M(MILLISECOND) | DTK_M(MICROSECOND))
#define DTK_DATE_M		(DTK_M(YEAR) | DTK_M(MONTH) | DTK_M(DAY))
#define DTK_TIME_M		(DTK_M(HOUR) | DTK_M(MINUTE) | DTK_ALL_SECS_M)

/*
 * Working buffer size for input and output of interval, timestamp, etc.
 * Inputs that need more working space will be rejected early.  Longer outputs
 * will overrun buffers, so this must suffice for all possible output.  As of
 * this writing, interval_out() needs the most space at ~90 bytes.
 */
#define MAXDATELEN		128
/* maximum possible number of fields in a date string */
#define MAXDATEFIELDS	25
/* only this many chars are stored in datetktbl */
#define TOKMAXLEN		10

/* keep this struct small; it gets used a lot */
typedef struct
{
	char		token[TOKMAXLEN + 1];	/* always NUL-terminated */
	char		type;			/* see field type codes above */
	int32_t		value;			/* meaning depends on type */
} datetkn;

#define HIGHBIT					(0x80)
#define IS_HIGHBIT_SET(ch)		((unsigned char)(ch) & HIGHBIT)

unsigned char
pg_tolower(unsigned char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		ch += 'a' - 'A';
	else if (IS_HIGHBIT_SET(ch) && isupper(ch))
		ch = tolower(ch);
	return ch;
}

/* FMODULO()
 * Macro to replace modf(), which is broken on some platforms.
 * t = input and remainder
 * q = integer part
 * u = divisor
 */
#define FMODULO(t,q,u) \
do { \
	(q) = (((t) < 0) ? ceil((t) / (u)) : floor((t) / (u))); \
	if ((q) != 0) (t) -= rint((q) * (u)); \
} while(0)

/* TMODULO()
 * Like FMODULO(), but work on the timestamp datatype (now always int64).
 * We assume that int64 follows the C99 semantics for division (negative
 * quotients truncate towards zero).
 */
#define TMODULO(t,q,u) \
do { \
	(q) = ((t) / (u)); \
	if ((q) != 0) (t) -= ((q) * (u)); \
} while(0)

/*
 * Date/time validation
 * Include check for leap year.
 */

extern const char *const months[];	/* months (3-char abbreviations) */
extern const char *const days[];	/* days (full names) */
extern const int day_tab[2][13];

/*
 * These are the rules for the Gregorian calendar, which was adopted in 1582.
 * However, we use this calculation for all prior years as well because the
 * SQL standard specifies use of the Gregorian calendar.  This prevents the
 * date 1500-02-29 from being stored, even though it is valid in the Julian
 * calendar.
 */
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))


/*
 * Datetime input parsing routines (ParseDateTime, DecodeDateTime, etc)
 * return zero or a positive value on success.  On failure, they return
 * one of these negative code values.  DateTimeParseError may be used to
 * produce a correct ereport.
 */
#define DTERR_BAD_FORMAT		(-1)
#define DTERR_FIELD_OVERFLOW	(-2)
#define DTERR_MD_FIELD_OVERFLOW (-3)	/* triggers hint about DateStyle */
#define DTERR_INTERVAL_OVERFLOW (-4)
#define DTERR_TZDISP_OVERFLOW	(-5)



static const datetkn datetktbl[] = {
	/* token, type, value */
	{EARLY, RESERV, DTK_EARLY}, /* "-infinity" reserved for "early time" */
	{DA_D, ADBC, AD},			/* "ad" for years > 0 */
	{"allballs", RESERV, DTK_ZULU}, /* 00:00:00 */
	{"am", AMPM, AM},
	{"apr", MONTH, 4},
	{"april", MONTH, 4},
	{"at", IGNORE_DTF, 0},		/* "at" (throwaway) */
	{"aug", MONTH, 8},
	{"august", MONTH, 8},
	{DB_C, ADBC, BC},			/* "bc" for years <= 0 */
	{"d", UNITS, DTK_DAY},		/* "day of month" for ISO input */
	{"dec", MONTH, 12},
	{"december", MONTH, 12},
	{"dow", UNITS, DTK_DOW},	/* day of week */
	{"doy", UNITS, DTK_DOY},	/* day of year */
	{"dst", DTZMOD, SECS_PER_HOUR},
	{EPOCH, RESERV, DTK_EPOCH}, /* "epoch" reserved for system epoch time */
	{"feb", MONTH, 2},
	{"february", MONTH, 2},
	{"fri", DOW, 5},
	{"friday", DOW, 5},
	{"h", UNITS, DTK_HOUR},		/* "hour" */
	{LATE, RESERV, DTK_LATE},	/* "infinity" reserved for "late time" */
	{"isodow", UNITS, DTK_ISODOW},	/* ISO day of week, Sunday == 7 */
	{"isoyear", UNITS, DTK_ISOYEAR},	/* year in terms of the ISO week date */
	{"j", UNITS, DTK_JULIAN},
	{"jan", MONTH, 1},
	{"january", MONTH, 1},
	{"jd", UNITS, DTK_JULIAN},
	{"jul", MONTH, 7},
	{"julian", UNITS, DTK_JULIAN},
	{"july", MONTH, 7},
	{"jun", MONTH, 6},
	{"june", MONTH, 6},
	{"m", UNITS, DTK_MONTH},	/* "month" for ISO input */
	{"mar", MONTH, 3},
	{"march", MONTH, 3},
	{"may", MONTH, 5},
	{"mm", UNITS, DTK_MINUTE},	/* "minute" for ISO input */
	{"mon", DOW, 1},
	{"monday", DOW, 1},
	{"nov", MONTH, 11},
	{"november", MONTH, 11},
	{NOW, RESERV, DTK_NOW},		/* current transaction time */
	{"oct", MONTH, 10},
	{"october", MONTH, 10},
	{"on", IGNORE_DTF, 0},		/* "on" (throwaway) */
	{"pm", AMPM, PM},
	{"s", UNITS, DTK_SECOND},	/* "seconds" for ISO input */
	{"sat", DOW, 6},
	{"saturday", DOW, 6},
	{"sep", MONTH, 9},
	{"sept", MONTH, 9},
	{"september", MONTH, 9},
	{"sun", DOW, 0},
	{"sunday", DOW, 0},
	{"t", ISOTIME, DTK_TIME},	/* Filler for ISO time fields */
	{"thu", DOW, 4},
	{"thur", DOW, 4},
	{"thurs", DOW, 4},
	{"thursday", DOW, 4},
	{TODAY, RESERV, DTK_TODAY}, /* midnight */
	{TOMORROW, RESERV, DTK_TOMORROW},	/* tomorrow midnight */
	{"tue", DOW, 2},
	{"tues", DOW, 2},
	{"tuesday", DOW, 2},
	{"wed", DOW, 3},
	{"wednesday", DOW, 3},
	{"weds", DOW, 3},
	{"y", UNITS, DTK_YEAR},		/* "year" for ISO input */
	{YESTERDAY, RESERV, DTK_YESTERDAY}	/* yesterday midnight */
};

static const int szdatetktbl = sizeof datetktbl / sizeof datetktbl[0];

/*
 * deltatktbl: same format as datetktbl, but holds keywords used to represent
 * time units (eg, for intervals, and for EXTRACT).
 */
static const datetkn deltatktbl[] = {
	/* token, type, value */
	{"@", IGNORE_DTF, 0},		/* postgres relative prefix */
	{DAGO, AGO, 0},				/* "ago" indicates negative time offset */
	{"c", UNITS, DTK_CENTURY},	/* "century" relative */
	{"cent", UNITS, DTK_CENTURY},	/* "century" relative */
	{"centuries", UNITS, DTK_CENTURY},	/* "centuries" relative */
	{DCENTURY, UNITS, DTK_CENTURY}, /* "century" relative */
	{"d", UNITS, DTK_DAY},		/* "day" relative */
	{DDAY, UNITS, DTK_DAY},		/* "day" relative */
	{"days", UNITS, DTK_DAY},	/* "days" relative */
	{"dec", UNITS, DTK_DECADE}, /* "decade" relative */
	{DDECADE, UNITS, DTK_DECADE},	/* "decade" relative */
	{"decades", UNITS, DTK_DECADE}, /* "decades" relative */
	{"decs", UNITS, DTK_DECADE},	/* "decades" relative */
	{"h", UNITS, DTK_HOUR},		/* "hour" relative */
	{DHOUR, UNITS, DTK_HOUR},	/* "hour" relative */
	{"hours", UNITS, DTK_HOUR}, /* "hours" relative */
	{"hr", UNITS, DTK_HOUR},	/* "hour" relative */
	{"hrs", UNITS, DTK_HOUR},	/* "hours" relative */
	{"m", UNITS, DTK_MINUTE},	/* "minute" relative */
	{"microsecon", UNITS, DTK_MICROSEC},	/* "microsecond" relative */
	{"mil", UNITS, DTK_MILLENNIUM}, /* "millennium" relative */
	{"millennia", UNITS, DTK_MILLENNIUM},	/* "millennia" relative */
	{DMILLENNIUM, UNITS, DTK_MILLENNIUM},	/* "millennium" relative */
	{"millisecon", UNITS, DTK_MILLISEC},	/* relative */
	{"mils", UNITS, DTK_MILLENNIUM},	/* "millennia" relative */
	{"min", UNITS, DTK_MINUTE}, /* "minute" relative */
	{"mins", UNITS, DTK_MINUTE},	/* "minutes" relative */
	{DMINUTE, UNITS, DTK_MINUTE},	/* "minute" relative */
	{"minutes", UNITS, DTK_MINUTE}, /* "minutes" relative */
	{"mon", UNITS, DTK_MONTH},	/* "months" relative */
	{"mons", UNITS, DTK_MONTH}, /* "months" relative */
	{DMONTH, UNITS, DTK_MONTH}, /* "month" relative */
	{"months", UNITS, DTK_MONTH},
	{"ms", UNITS, DTK_MILLISEC},
	{"msec", UNITS, DTK_MILLISEC},
	{DMILLISEC, UNITS, DTK_MILLISEC},
	{"mseconds", UNITS, DTK_MILLISEC},
	{"msecs", UNITS, DTK_MILLISEC},
	{"qtr", UNITS, DTK_QUARTER},	/* "quarter" relative */
	{DQUARTER, UNITS, DTK_QUARTER}, /* "quarter" relative */
	{"s", UNITS, DTK_SECOND},
	{"sec", UNITS, DTK_SECOND},
	{DSECOND, UNITS, DTK_SECOND},
	{"seconds", UNITS, DTK_SECOND},
	{"secs", UNITS, DTK_SECOND},
	{DTIMEZONE, UNITS, DTK_TZ}, /* "timezone" time offset */
	{"timezone_h", UNITS, DTK_TZ_HOUR}, /* timezone hour units */
	{"timezone_m", UNITS, DTK_TZ_MINUTE},	/* timezone minutes units */
	{"us", UNITS, DTK_MICROSEC},	/* "microsecond" relative */
	{"usec", UNITS, DTK_MICROSEC},	/* "microsecond" relative */
	{DMICROSEC, UNITS, DTK_MICROSEC},	/* "microsecond" relative */
	{"useconds", UNITS, DTK_MICROSEC},	/* "microseconds" relative */
	{"usecs", UNITS, DTK_MICROSEC}, /* "microseconds" relative */
	{"w", UNITS, DTK_WEEK},		/* "week" relative */
	{DWEEK, UNITS, DTK_WEEK},	/* "week" relative */
	{"weeks", UNITS, DTK_WEEK}, /* "weeks" relative */
	{"y", UNITS, DTK_YEAR},		/* "year" relative */
	{DYEAR, UNITS, DTK_YEAR},	/* "year" relative */
	{"years", UNITS, DTK_YEAR}, /* "years" relative */
	{"yr", UNITS, DTK_YEAR},	/* "year" relative */
	{"yrs", UNITS, DTK_YEAR}	/* "years" relative */
};

static const datetkn *datecache[MAXDATEFIELDS] = {NULL};




static const datetkn *
datebsearch(const char *key, const datetkn *base, int nel)
{
	if (nel > 0)
	{
		const datetkn *last = base + nel - 1,
				   *position;
		int			result;

		while (last >= base)
		{
			position = base + ((last - base) >> 1);
			/* precheck the first character for a bit of extra speed */
			result = (int) key[0] - (int) position->token[0];
			if (result == 0)
			{
				/* use strncmp so that we match truncated tokens */
				result = strncmp(key, position->token, TOKMAXLEN);
				if (result == 0)
					return position;
			}
			if (result < 0)
				last = position - 1;
			else
				base = position + 1;
		}
	}
	return NULL;
}

int
ParseDateTime(const char *timestr, char *workbuf, size_t buflen,
			  char **field, int *ftype, int maxfields, int *numfields)
{
	int			nf = 0;
	const char *cp = timestr;
	char	   *bufp = workbuf;
	const char *bufend = workbuf + buflen;

	/*
	 * Set the character pointed-to by "bufptr" to "newchar", and increment
	 * "bufptr". "end" gives the end of the buffer -- we return an error if
	 * there is no space left to append a character to the buffer. Note that
	 * "bufptr" is evaluated twice.
	 */
#define APPEND_CHAR(bufptr, end, newchar)		\
	do											\
	{											\
		if (((bufptr) + 1) >= (end))			\
			return DTERR_BAD_FORMAT;			\
		*(bufptr)++ = newchar;					\
	} while (0)

	/* outer loop through fields */
	while (*cp != '\0')
	{
		/* Ignore spaces between fields */
		if (isspace((unsigned char) *cp))
		{
			cp++;
			continue;
		}

		/* Record start of current field */
		if (nf >= maxfields)
			return DTERR_BAD_FORMAT;
		field[nf] = bufp;

		/* leading digit? then date or time */
		if (isdigit((unsigned char) *cp))
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			while (isdigit((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, *cp++);

			/* time field? */
			if (*cp == ':')
			{
				ftype[nf] = DTK_TIME;
				APPEND_CHAR(bufp, bufend, *cp++);
				while (isdigit((unsigned char) *cp) ||
					   (*cp == ':') || (*cp == '.'))
					APPEND_CHAR(bufp, bufend, *cp++);
			}
			/* date field? allow embedded text month */
			else if (*cp == '-' || *cp == '/' || *cp == '.')
			{
				/* save delimiting character to use later */
				char		delim = *cp;

				APPEND_CHAR(bufp, bufend, *cp++);
				/* second field is all digits? then no embedded text month */
				if (isdigit((unsigned char) *cp))
				{
					ftype[nf] = ((delim == '.') ? DTK_NUMBER : DTK_DATE);
					while (isdigit((unsigned char) *cp))
						APPEND_CHAR(bufp, bufend, *cp++);

					/*
					 * insist that the delimiters match to get a three-field
					 * date.
					 */
					if (*cp == delim)
					{
						ftype[nf] = DTK_DATE;
						APPEND_CHAR(bufp, bufend, *cp++);
						while (isdigit((unsigned char) *cp) || *cp == delim)
							APPEND_CHAR(bufp, bufend, *cp++);
					}
				}
				else
				{
					ftype[nf] = DTK_DATE;
					while (isalnum((unsigned char) *cp) || *cp == delim)
						APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				}
			}

			/*
			 * otherwise, number only and will determine year, month, day, or
			 * concatenated fields later...
			 */
			else
				ftype[nf] = DTK_NUMBER;
		}
		/* Leading decimal point? Then fractional seconds... */
		else if (*cp == '.')
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			while (isdigit((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, *cp++);

			ftype[nf] = DTK_NUMBER;
		}

		/*
		 * text? then date string, month, day of week, special, or timezone
		 */
		else if (isalpha((unsigned char) *cp))
		{
			bool		is_date;

			ftype[nf] = DTK_STRING;
			APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
			while (isalpha((unsigned char) *cp))
				APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));

			/*
			 * Dates can have embedded '-', '/', or '.' separators.  It could
			 * also be a timezone name containing embedded '/', '+', '-', '_',
			 * or ':' (but '_' or ':' can't be the first punctuation). If the
			 * next character is a digit or '+', we need to check whether what
			 * we have so far is a recognized non-timezone keyword --- if so,
			 * don't believe that this is the start of a timezone.
			 */
			is_date = false;
			if (*cp == '-' || *cp == '/' || *cp == '.')
				is_date = true;
			else if (*cp == '+' || isdigit((unsigned char) *cp))
			{
				*bufp = '\0';	/* null-terminate current field value */
				/* we need search only the core token table, not TZ names */
				if (datebsearch(field[nf], datetktbl, szdatetktbl) == NULL)
					is_date = true;
			}
			if (is_date)
			{
				ftype[nf] = DTK_DATE;
				do
				{
					APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				} while (*cp == '+' || *cp == '-' ||
						 *cp == '/' || *cp == '_' ||
						 *cp == '.' || *cp == ':' ||
						 isalnum((unsigned char) *cp));
			}
		}
		/* sign? then special or numeric timezone */
		else if (*cp == '+' || *cp == '-')
		{
			APPEND_CHAR(bufp, bufend, *cp++);
			/* soak up leading whitespace */
			while (isspace((unsigned char) *cp))
				cp++;
			/* numeric timezone? */
			/* note that "DTK_TZ" could also be a signed float or yyyy-mm */
			if (isdigit((unsigned char) *cp))
			{
				ftype[nf] = DTK_TZ;
				APPEND_CHAR(bufp, bufend, *cp++);
				while (isdigit((unsigned char) *cp) ||
					   *cp == ':' || *cp == '.' || *cp == '-')
					APPEND_CHAR(bufp, bufend, *cp++);
			}
			/* special? */
			else if (isalpha((unsigned char) *cp))
			{
				ftype[nf] = DTK_SPECIAL;
				APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
				while (isalpha((unsigned char) *cp))
					APPEND_CHAR(bufp, bufend, pg_tolower((unsigned char) *cp++));
			}
			/* otherwise something wrong... */
			else
				return DTERR_BAD_FORMAT;
		}
		/* ignore other punctuation but use as delimiter */
		else if (ispunct((unsigned char) *cp))
		{
			cp++;
			continue;
		}
		/* otherwise, something is not right... */
		else
			return DTERR_BAD_FORMAT;

		/* force in a delimiter after each field */
		*bufp++ = '\0';
		nf++;
	}

	*numfields = nf;

	return 0;
}

typedef int64_t pg_time_t;

struct pg_tm
{
	int			tm_sec;
	int			tm_min;
	int			tm_hour;
	int			tm_mday;
	int			tm_mon;			/* origin 1, not 0! */
	int			tm_year;		/* relative to 1900 */
	int			tm_wday;
	int			tm_yday;
	int			tm_isdst;
	long int	tm_gmtoff;
	const char *tm_zone;
};


static int
DecodeNumberField(int len, char *str, int fmask,
				  int *tmask, struct pg_tm *tm, fsec_t *fsec, bool *is2digits)
{
	char	   *cp;

	/*
	 * Have a decimal point? Then this is a date or something with a seconds
	 * field...
	 */
	if ((cp = strchr(str, '.')) != NULL)
	{
		char		fstr[7];
		int			i;

		cp++;

		/*
		 * OK, we have at most six digits to care about. Let's construct a
		 * string with those digits, zero-padded on the right, and then do the
		 * conversion to an integer.
		 *
		 * XXX This truncates the seventh digit, unlike rounding it as the
		 * backend does.
		 */
		for (i = 0; i < 6; i++)
			fstr[i] = *cp != '\0' ? *cp++ : '0';
		fstr[i] = '\0';
		*fsec = strtol(fstr, NULL, 10);
		*cp = '\0';
		len = strlen(str);
	}
	/* No decimal point and no complete date yet? */
	else if ((fmask & DTK_DATE_M) != DTK_DATE_M)
	{
		/* yyyymmdd? */
		if (len == 8)
		{
			*tmask = DTK_DATE_M;

			tm->tm_mday = atoi(str + 6);
			*(str + 6) = '\0';
			tm->tm_mon = atoi(str + 4);
			*(str + 4) = '\0';
			tm->tm_year = atoi(str + 0);

			return DTK_DATE;
		}
		/* yymmdd? */
		else if (len == 6)
		{
			*tmask = DTK_DATE_M;
			tm->tm_mday = atoi(str + 4);
			*(str + 4) = '\0';
			tm->tm_mon = atoi(str + 2);
			*(str + 2) = '\0';
			tm->tm_year = atoi(str + 0);
			*is2digits = true;

			return DTK_DATE;
		}
		/* yyddd? */
		else if (len == 5)
		{
			*tmask = DTK_DATE_M;
			tm->tm_mday = atoi(str + 2);
			*(str + 2) = '\0';
			tm->tm_mon = 1;
			tm->tm_year = atoi(str + 0);
			*is2digits = true;

			return DTK_DATE;
		}
	}

	/* not all time fields are specified? */
	if ((fmask & DTK_TIME_M) != DTK_TIME_M)
	{
		/* hhmmss */
		if (len == 6)
		{
			*tmask = DTK_TIME_M;
			tm->tm_sec = atoi(str + 4);
			*(str + 4) = '\0';
			tm->tm_min = atoi(str + 2);
			*(str + 2) = '\0';
			tm->tm_hour = atoi(str + 0);

			return DTK_TIME;
		}
		/* hhmm? */
		else if (len == 4)
		{
			*tmask = DTK_TIME_M;
			tm->tm_sec = 0;
			tm->tm_min = atoi(str + 2);
			*(str + 2) = '\0';
			tm->tm_hour = atoi(str + 0);

			return DTK_TIME;
		}
	}

	return -1;
}	

int
date2j(int y, int m, int d)
{
	int			julian;
	int			century;

	if (m > 2)
	{
		m += 1;
		y += 4800;
	}
	else
	{
		m += 13;
		y += 4799;
	}

	century = y / 100;
	julian = y * 365 - 32167;
	julian += y / 4 - century + century / 4;
	julian += 7834 * m / 256 + d;

	return julian;
}								/* date2j() */

void
j2date(int jd, int *year, int *month, int *day)
{
	unsigned int julian;
	unsigned int quad;
	unsigned int extra;
	int			y;

	julian = jd;
	julian += 32044;
	quad = julian / 146097;
	extra = (julian - quad * 146097) * 4 + 3;
	julian += 60 + quad * 3 + extra / 146097;
	quad = julian / 1461;
	julian -= quad * 1461;
	y = julian * 4 / 1461;
	julian = ((y != 0) ? (julian + 305) % 365 : (julian + 306) % 366) + 123;
	y += quad * 4;
	*year = y - 4800;
	quad = julian * 2141 / 65536;
	*day = julian - 7834 * quad / 256;
	*month = (quad + 10) % 12 + 1;
}	

static int
DecodeNumber(int flen, char *str, int fmask,
			 int *tmask, struct pg_tm *tm, fsec_t *fsec, bool *is2digits, bool EuroDates)
{
	int			val;
	char	   *cp;

	*tmask = 0;

	val = strtol(str, &cp, 10);
	if (cp == str)
		return -1;

	if (*cp == '.')
	{
		/*
		 * More than two digits? Then could be a date or a run-together time:
		 * 2001.360 20011225 040506.789
		 */
		if (cp - str > 2)
			return DecodeNumberField(flen, str, (fmask | DTK_DATE_M),
									 tmask, tm, fsec, is2digits);

		*fsec = strtod(cp, &cp);
		if (*cp != '\0')
			return -1;
	}
	else if (*cp != '\0')
		return -1;

	/* Special case day of year? */
	if (flen == 3 && (fmask & DTK_M(YEAR)) && val >= 1 && val <= 366)
	{
		*tmask = (DTK_M(DOY) | DTK_M(MONTH) | DTK_M(DAY));
		tm->tm_yday = val;
		j2date(date2j(tm->tm_year, 1, 1) + tm->tm_yday - 1,
			   &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
	}

	/***
	 * Enough digits to be unequivocal year? Used to test for 4 digits or
	 * more, but we now test first for a three-digit doy so anything
	 * bigger than two digits had better be an explicit year.
	 * - thomas 1999-01-09
	 * Back to requiring a 4 digit year. We accept a two digit
	 * year farther down. - thomas 2000-03-28
	 ***/
	else if (flen >= 4)
	{
		*tmask = DTK_M(YEAR);

		/* already have a year? then see if we can substitute... */
		if ((fmask & DTK_M(YEAR)) && !(fmask & DTK_M(DAY)) &&
			tm->tm_year >= 1 && tm->tm_year <= 31)
		{
			tm->tm_mday = tm->tm_year;
			*tmask = DTK_M(DAY);
		}

		tm->tm_year = val;
	}

	/* already have year? then could be month */
	else if ((fmask & DTK_M(YEAR)) && !(fmask & DTK_M(MONTH)) && val >= 1 && val <= MONTHS_PER_YEAR)
	{
		*tmask = DTK_M(MONTH);
		tm->tm_mon = val;
	}
	/* no year and EuroDates enabled? then could be day */
	else if ((EuroDates || (fmask & DTK_M(MONTH))) &&
			 !(fmask & DTK_M(YEAR)) && !(fmask & DTK_M(DAY)) &&
			 val >= 1 && val <= 31)
	{
		*tmask = DTK_M(DAY);
		tm->tm_mday = val;
	}
	else if (!(fmask & DTK_M(MONTH)) && val >= 1 && val <= MONTHS_PER_YEAR)
	{
		*tmask = DTK_M(MONTH);
		tm->tm_mon = val;
	}
	else if (!(fmask & DTK_M(DAY)) && val >= 1 && val <= 31)
	{
		*tmask = DTK_M(DAY);
		tm->tm_mday = val;
	}

	/*
	 * Check for 2 or 4 or more digits, but currently we reach here only if
	 * two digits. - thomas 2000-03-28
	 */
	else if (!(fmask & DTK_M(YEAR)) && (flen >= 4 || flen == 2))
	{
		*tmask = DTK_M(YEAR);
		tm->tm_year = val;

		/* adjust ONLY if exactly two digits... */
		*is2digits = (flen == 2);
	}
	else
		return -1;

	return 0;
}	

static int
DecodeSpecial(int field, char *lowtoken, int *val)
{
	int			type;
	const datetkn *tp;

	/* use strncmp so that we match truncated tokens */
	if (datecache[field] != NULL &&
		strncmp(lowtoken, datecache[field]->token, TOKMAXLEN) == 0)
		tp = datecache[field];
	else
	{
		tp = NULL;
		if (!tp)
			tp = datebsearch(lowtoken, datetktbl, szdatetktbl);
	}
	datecache[field] = tp;
	if (tp == NULL)
	{
		type = UNKNOWN_FIELD;
		*val = 0;
	}
	else
	{
		type = tp->type;
		*val = tp->value;
	}

	return type;
}		

static int
DecodeDate(char *str, int fmask, int *tmask, struct pg_tm *tm, bool EuroDates)
{
	fsec_t		fsec;

	int			nf = 0;
	int			i,
				len;
	bool		bc = false;
	bool		is2digits = false;
	int			type,
				val,
				dmask = 0;
	char	   *field[MAXDATEFIELDS];

	/* parse this string... */
	while (*str != '\0' && nf < MAXDATEFIELDS)
	{
		/* skip field separators */
		while (!isalnum((unsigned char) *str))
			str++;

		field[nf] = str;
		if (isdigit((unsigned char) *str))
		{
			while (isdigit((unsigned char) *str))
				str++;
		}
		else if (isalpha((unsigned char) *str))
		{
			while (isalpha((unsigned char) *str))
				str++;
		}

		/* Just get rid of any non-digit, non-alpha characters... */
		if (*str != '\0')
			*str++ = '\0';
		nf++;
	}

#if 0
	/* don't allow too many fields */
	if (nf > 3)
		return -1;
#endif

	*tmask = 0;

	/* look first for text fields, since that will be unambiguous month */
	for (i = 0; i < nf; i++)
	{
		if (isalpha((unsigned char) *field[i]))
		{
			type = DecodeSpecial(i, field[i], &val);
			if (type == IGNORE_DTF)
				continue;

			dmask = DTK_M(type);
			switch (type)
			{
				case MONTH:
					tm->tm_mon = val;
					break;

				case ADBC:
					bc = (val == BC);
					break;

				default:
					return -1;
			}
			if (fmask & dmask)
				return -1;

			fmask |= dmask;
			*tmask |= dmask;

			/* mark this field as being completed */
			field[i] = NULL;
		}
	}

	/* now pick up remaining numeric fields */
	for (i = 0; i < nf; i++)
	{
		if (field[i] == NULL)
			continue;

		if ((len = strlen(field[i])) <= 0)
			return -1;

		if (DecodeNumber(len, field[i], fmask, &dmask, tm, &fsec, &is2digits, EuroDates) != 0)
			return -1;

		if (fmask & dmask)
			return -1;

		fmask |= dmask;
		*tmask |= dmask;
	}

	if ((fmask & ~(DTK_M(DOY) | DTK_M(TZ))) != DTK_DATE_M)
		return -1;

	/* there is no year zero in AD/BC notation; i.e. "1 BC" == year 0 */
	if (bc)
	{
		if (tm->tm_year > 0)
			tm->tm_year = -(tm->tm_year - 1);
		else
			return -1;
	}
	else if (is2digits)
	{
		if (tm->tm_year < 70)
			tm->tm_year += 2000;
		else if (tm->tm_year < 100)
			tm->tm_year += 1900;
	}

	return 0;
}	

int DecodeTime(char *str, int *tmask, struct pg_tm *tm, fsec_t *fsec)
{
	char	   *cp;

	*tmask = DTK_TIME_M;

	tm->tm_hour = strtol(str, &cp, 10);
	if (*cp != ':')
		return -1;
	str = cp + 1;
	tm->tm_min = strtol(str, &cp, 10);
	if (*cp == '\0')
	{
		tm->tm_sec = 0;
		*fsec = 0;
	}
	else if (*cp != ':')
		return -1;
	else
	{
		str = cp + 1;
		tm->tm_sec = strtol(str, &cp, 10);
		if (*cp == '\0')
			*fsec = 0;
		else if (*cp == '.')
		{
			char		fstr[7];
			int			i;

			cp++;

			/*
			 * OK, we have at most six digits to care about. Let's construct a
			 * string with those digits, zero-padded on the right, and then do
			 * the conversion to an integer.
			 *
			 * XXX This truncates the seventh digit, unlike rounding it as the
			 * backend does.
			 */
			for (i = 0; i < 6; i++)
				fstr[i] = *cp != '\0' ? *cp++ : '0';
			fstr[i] = '\0';
			*fsec = strtol(fstr, &cp, 10);
			if (*cp != '\0')
				return -1;
		}
		else
			return -1;
	}

	/* do a sanity check */
	if (tm->tm_hour < 0 || tm->tm_min < 0 || tm->tm_min > 59 ||
		tm->tm_sec < 0 || tm->tm_sec > 59 || *fsec >= USECS_PER_SEC)
		return -1;

	return 0;
}	




static Timestamp
dt2local(Timestamp dt, int tz)
{
	dt -= (tz * USECS_PER_SEC);
	return dt;
}

static TimeOffset time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
    return (((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec + fsec;
}


int
tm2timestamp(struct pg_tm *tm, fsec_t fsec, int *tzp, Timestamp *result)
{
	TimeOffset	date;
	TimeOffset	time;


	date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;
	time = time2t(tm->tm_hour, tm->tm_min, tm->tm_sec, fsec);

	*result = date * USECS_PER_DAY + time;
	/* check for major overflow */
	if ((*result - time) / USECS_PER_DAY != date)
	{
		*result = 0;			/* keep compiler quiet */
		return -1;
	}
	/* check for just-barely overflow (okay except time-of-day wraps) */
	/* caution: we want to allow 1999-12-31 24:00:00 */
	if ((*result < 0 && date > 0) ||
		(*result > 0 && date < -1))
	{
		*result = 0;			/* keep compiler quiet */
		return -1;
	}
	if (tzp != NULL)
		*result = dt2local(*result, -(*tzp));

	/* final range check catches just-out-of-range timestamps */
	if (!IS_VALID_TIMESTAMP(*result))
	{
		*result = 0;			/* keep compiler quiet */
		return -1;
	}

	return 0;
}

const int	day_tab[2][13] =
{
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0},
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0}
};


int
DecodeDateTime(char **field, int *ftype, int nf,
			   int *dtype, struct pg_tm *tm, fsec_t *fsec, bool EuroDates)
{
	int			fmask = 0,
				tmask,
				type;
	int			ptype = 0;		/* "prefix type" for ISO y2001m02d04 format */
	int			i;
	int			val;
	int			mer = HR24;
	bool		haveTextMonth = false;
	bool		is2digits = false;
	bool		bc = false;
	int			t = 0;
	int		   *tzp = &t;

	/***
	 * We'll insist on at least all of the date fields, but initialize the
	 * remaining fields in case they are not set later...
	 ***/
	*dtype = DTK_DATE;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	*fsec = 0;
	/* don't know daylight savings time status apriori */
	tm->tm_isdst = -1;
	if (tzp != NULL)
		*tzp = 0;

	for (i = 0; i < nf; i++)
	{
		switch (ftype[i])
		{
			case DTK_DATE:
				if (((fmask & DTK_DATE_M) == DTK_DATE_M)
						 || (ptype != 0))
				{
					/* No time zone accepted? Then quit... */
					if (tzp == NULL)
						return -1;

					if (isdigit((unsigned char) *field[i]) || ptype != 0)
					{
						char	   *cp;

						if (ptype != 0)
						{
							/* Sanity check; should not fail this test */
							if (ptype != DTK_TIME)
								return -1;
							ptype = 0;
						}

						/*
						 * Starts with a digit but we already have a time
						 * field? Then we are in trouble with a date and time
						 * already...
						 */
						if ((fmask & DTK_TIME_M) == DTK_TIME_M)
							return -1;

						if ((cp = strchr(field[i], '-')) == NULL)
							return -1;

						*cp = '\0';

						/*
						 * Then read the rest of the field as a concatenated
						 * time
						 */
						if ((ftype[i] = DecodeNumberField(strlen(field[i]), field[i], fmask,
														  &tmask, tm, fsec, &is2digits)) < 0)
							return -1;

						/*
						 * modify tmask after returning from
						 * DecodeNumberField()
						 */
						tmask |= DTK_M(TZ);
					}
					
				}
				else if (DecodeDate(field[i], fmask, &tmask, tm, EuroDates) != 0)
					return -1;
				break;

			case DTK_TIME:
				if (DecodeTime(field[i], &tmask, tm, fsec) != 0)
					return -1;

				/*
				 * Check upper limit on hours; other limits checked in
				 * DecodeTime()
				 */
				/* test for > 24:00:00 */
				if (tm->tm_hour > 24 ||
					(tm->tm_hour == 24 && (tm->tm_min > 0 || tm->tm_sec > 0)))
					return -1;
				break;

			default:
				return -1;
		}

		if (tmask & fmask)
			return -1;
		fmask |= tmask;
	}

	/* there is no year zero in AD/BC notation; i.e. "1 BC" == year 0 */
	if (bc)
	{
		if (tm->tm_year > 0)
			tm->tm_year = -(tm->tm_year - 1);
		else
			return -1;
	}
	else if (is2digits)
	{
		if (tm->tm_year < 70)
			tm->tm_year += 2000;
		else if (tm->tm_year < 100)
			tm->tm_year += 1900;
	}

	if (mer != HR24 && tm->tm_hour > 12)
		return -1;
	if (mer == AM && tm->tm_hour == 12)
		tm->tm_hour = 0;
	else if (mer == PM && tm->tm_hour != 12)
		tm->tm_hour += 12;

	/* do additional checking for full date specs... */
	if (*dtype == DTK_DATE)
	{
		if ((fmask & DTK_DATE_M) != DTK_DATE_M)
			return ((fmask & DTK_TIME_M) == DTK_TIME_M) ? 1 : -1;

		/*
		 * check for valid day of month, now that we know for sure the month
		 * and year...
		 */
		if (tm->tm_mday < 1 || tm->tm_mday > day_tab[isleap(tm->tm_year)][tm->tm_mon - 1])
			return -1;

		/*
		 * backend tried to find local timezone here but we don't use the
		 * result afterwards anyway so we only check for this error: daylight
		 * savings time modifier but no standard timezone?
		 */
		if ((fmask & DTK_DATE_M) == DTK_DATE_M && tzp != NULL && !(fmask & DTK_M(TZ)) && (fmask & DTK_M(DTZMOD)))
			return -1;
	}

	return 0;
}								/* DecodeDateTime() */
