/*	$Id: test-debug-write-long.c,v 1.1 2020/07/21 10:44:27 kristaps Exp $ */
/*
 * Copyright (c) 2020 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "../config.h"

#if HAVE_ERR
# include <err.h>
#endif
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>

#include "../kcgi.h"
#include "regress.h"

static char log[33] = "/tmp/test-debug-write.XXXXXXXXXX";

static const char *data = "foo=0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456789"
	"0123456";

static int
parent(CURL *curl)
{

	curl_easy_setopt(curl, CURLOPT_URL, 
		"http://localhost:17123/");
	return curl_easy_perform(curl) == CURLE_OK;
}

static int
child(void)
{
	struct kreq	 r;
	const char 	*page = "index";
	enum kcgi_err	 er;

	if (!kutil_openlog(log))
		return 0;

	er = khttp_parsex
		(&r, ksuffixmap, kmimetypes, KMIME__MAX, 
		 NULL, 0, &page, 1, KMIME_TEXT_HTML,
		 0, NULL, NULL, KREQ_DEBUG_WRITE, NULL);
	if (er != KCGI_OK)
		return 0;

	khttp_head(&r, kresps[KRESP_STATUS], 
		"%s", khttps[KHTTP_200]);
	khttp_head(&r, kresps[KRESP_CONTENT_TYPE], 
		"%s", kmimetypes[KMIME_TEXT_HTML]);
	khttp_body(&r);
	khttp_puts(&r, data);
	khttp_free(&r);
	return 1;
}

int
main(int argc, char *argv[])
{
	int		 fd = -1, rc = 1;
	FILE		*f = NULL;
	char		*line = NULL;
	const char	*cp;
	size_t		 linesize = 0, lineno = 0;
	ssize_t		 linelen;
	struct log_line	 log_line;

	if ((fd = mkstemp(log)) == -1)
		err(1, "%s", log);

	if (!regress_cgi(parent, child))
		goto out;

	if ((f = fdopen(fd, "r")) == NULL) {
		warn("%s", log);
		goto out;
	}

	for (;;) {
		if ((linelen = getline(&line, &linesize, f)) == -1)
			break;
		if (!log_line_parse(line, &log_line))
			goto out;
		if (strcmp(log_line.level, "INFO"))
			continue;

		/* Extract after pid-tx. */

		if ((cp = strchr(log_line.umsg, ':')) == NULL)
			goto out;
		cp++;
		while (*cp == ' ')
			cp++;

		/* Examine line-by-line. */

		switch (lineno++) {
		case 0:
			if (strcmp(cp, "Status: 200 OK\\r\n"))
				goto out;
			break;
		case 1:
			if (strcmp(cp, "Content-Type: text/html\\r\n"))
				goto out;
			break;
		case 2:
			if (strcmp(cp, "\\r\n"))
				goto out;
			break;
		case 3:
			if (strcmp(cp, "foo="
			    "012345678901234567890123456789"
			    "012345678901234567890123456789"
			    "0123456789012345...\n"))
				goto out;
			break;
		case 4:
			if (strcmp(cp, "6789"
			    "012345678901234567890123456789"
			    "012345678901234567890123456789"
			    "0123456789012345...\n"))
				goto out;
			break;
		case 5:
			if (strcmp(cp, "6\n"))
				goto out;
			break;
		case 6:
			/* Ignore. */
			break;
		default:
			break;
		}
	}

	if (ferror(f) || lineno != 7)
		goto out;

	rc = 0;
out:
	if (f != NULL)
		fclose(f);
	else if (fd != -1)
		close(fd);

	free(line);
	unlink(log);
	return rc;
}
