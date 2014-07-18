#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "unicode.h"
#include "edit.h"
#include "win.h"
#include "exec.h"

typedef struct ecmd ECmd;
struct ecmd {
	char *name;
	int (*f)(W *, EBuf *, unsigned);
};

static ECmd *lookup(Buf *, unsigned, unsigned *);
static unsigned skipb(Buf *, unsigned, int);
static int get(W *, EBuf *, unsigned);
static int look(W *, EBuf *, unsigned);
static int run(W *, EBuf *, unsigned);

ECmd etab[] = {
	{ "Get", get },
	{ "Look", look },
	{ 0, run },
};


/* ex_run - Execute a command in the current window at
 * position [p0].  The command is first searched among
 * the list of builtins, if not found, it is run in a shell.
 */
int
ex_run(unsigned p0)
{
	extern W *curwin;
	unsigned p1;
	ECmd *e;

	e = lookup(&curwin->eb->b, p0, &p1);
	if (e && e->f(win_text(curwin), curwin->eb, p1))
	if (win_text(curwin) != curwin)
		curwin = win_tag_toggle(curwin);

	return 0;
}

/* ex_look - Look for a string [s] in window [w] and jump
 * to the first match after the cursor position.  The caller
 * is responsible to free the [s] buffer.
 */
int
ex_look(W *w, Rune *s, unsigned n)
{
	unsigned p;

	p = eb_look(w->eb, w->cu+1, s, n);
	if (p == -1u)
		p = eb_look(w->eb, 0, s, n);
	if (p != -1u) {
		w->cu = p;
		eb_setmark(w->eb, SelBeg, p);
		eb_setmark(w->eb, SelEnd, p+n);
	}

	return p == -1u;
}


/* static functions */

static int
risspace(Rune r)
{
	return risascii(r) && isspace(r);
}

static unsigned
skipb(Buf *b, unsigned p, int dir)
{
	assert(dir == -1 || dir == +1);
	while (risspace(buf_get(b, p)))
		p += dir;
	return p;
}

static ECmd *
lookup(Buf *b, unsigned p0, unsigned *p1)
{
	Rune r;
	char *s;
	ECmd *e;

	p0 = skipb(b, buf_bol(b, p0), +1);

	for (e = etab; (s = e->name); e++) {
		*p1 = p0;
		do {
			r = buf_get(b, *p1);
			if (!*s && (risspace(r) || r == '\n')) {
				*p1 = skipb(b, *p1, +1);
				return e;
			}
			(*p1)++;
		} while (risascii(r) && r == (Rune)*s++);
	}

	*p1 = p0;
	return e;
}


/* builtin commands */

static int
get(W *w, EBuf *eb, unsigned p0)
{
	Rune r;
	unsigned p1;
	char a[1024], *f, *p;
	long ln;

	ln = 1;
	p1 = 1 + skipb(&eb->b, buf_eol(&eb->b, p0) - 1, -1);

	if (p0 < p1) {
		p = a;
		for (; p0 < p1; p0++) {
			r = buf_get(&eb->b, p0);
			assert ((unsigned)utf8_rune_len(r) < sizeof a - (p - a));
			p += utf8_encode_rune(r, (unsigned char *)p, 8);
		}
		*p = 0;
		if ((p = strchr(a, ':'))) {
			*p = 0;
			ln = strtol(p+1, 0, 10);
			if (ln > INT_MAX || ln < 0)
				ln = 0;
		}
		f = malloc(strlen(a)+1);
		memcpy(f, a, strlen(a)+1);
	} else
		f = w->eb->path;

	if (!eb_read(w->eb, f)) {
		w->cu = buf_setlc(&w->eb->b, ln-1, 0);
		return 1;
	} else
		return 0;
}

static int
look(W *w, EBuf *eb, unsigned p0)
{
	YBuf b = {0,0,0,0};
	unsigned p1;

	p1 = 1 + skipb(&eb->b, buf_eol(&eb->b, p0) - 1, -1);
	if (p1 == p0)
		return 0;

	eb_yank(eb, p0, p1, &b);
	ex_look(w, b.r, b.nr);
	free(b.r);
	return 1;
}

struct Run {
	EBuf *eb;
	YBuf *o;
	unsigned p;
	unsigned no;
};

static int
runev(int flag, void *data)
{
	struct Run *r;

	r = data;

	/* commit changes (if any) here */
	/* set selection and commit changes if text was added */
	return 0;
}

static int
run(W *w, EBuf *eb, unsigned p0)
{
	/* clear (and possibly delete) selection, get the "insertion" position and set a mark for it in the edit buffer (Acme does not do this, it just stores an offset)

	what happens when eb is deleted/changed during the command execution?
		refcount ebs and make eb_free free the data and have eb contain simply the refcount
		when a dummy eb is detected in the callback, just abort the IO operation
	*/
	return 0;
}
