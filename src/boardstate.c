#include "boardstate.h"

#ifdef __PORT_sm83
#include <rand.h>
#else
#include <stdlib.h>
#endif

void
mksolvable(char * board)
{
	char parity = 0;
	char i;
	char j;
	char k;
	char x;
	/* fill */
	for (i = 16; i; --i)
	{
		k = i - 1;
		board[k] = k;
	}
	/* shuffle */
	for (i = 15; i; --i)
	{
		j = rand() % (i + 1);
		x = board[i];
		board[i] = board[j];
		board[j] = x;
	}
	/* check solvability */
	for (i = 16; i; --i)
	{
		k = i - 1;
		x = board[k];
		if (!x)
		{
			parity ^= !!((k>>2) & 1);
			continue;
		}
		for (j = k; j; --j)
		{
			parity ^= (board[j-1] > x);
		}
	}
	if (!parity)
	{
		i = 15;
		j = 14;
		if (!board[i]) { --i; --j; }
		if (!board[j]) { --j; }
		x = board[i];
		board[i] = board[j];
		board[j] = x;
	}
}
