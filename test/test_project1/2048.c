/*
	=========================================================================
 		Name			: 2048-C
 		Author			: Isuru Bandaranayake
 		Date			: 23/01/2019
 		Description		: CLI version of 2048 for Linux using C
	=========================================================================
*/

#include <kernel.h>

unsigned int simple_itoa(
    long value, unsigned int radix, unsigned int uppercase,
    unsigned int unsig, char *buffer, unsigned int zero_pad)
{
    char *pbuffer = buffer;
    int negative  = 0;
    unsigned int i, len;

    /* No support for unusual radixes. */
    if (radix > 16) return 0;

    if (value < 0 && !unsig) {
        negative = 1;
        value    = -value;
    }

    /* This builds the string back to front ... */
    do {
        int digit = value % radix;
        *(pbuffer++) =
            (digit < 10 ? '0' + digit :
                          (uppercase ? 'A' : 'a') + digit - 10);
        value /= radix;
    } while (value > 0);

    for (i = (pbuffer - buffer); i < zero_pad; i++)
        *(pbuffer++) = '0';

    if (negative) *(pbuffer++) = '-';

    *(pbuffer) = '\0';

    /* ... now we reverse it (could do it recursively but will
     * conserve the stack space) */
    len = (pbuffer - buffer);
    for (i = 0; i < len / 2; i++) {
        char j              = buffer[i];
        buffer[i]           = buffer[len - i - 1];
        buffer[len - i - 1] = j;
    }

    return len;
}

void print_value(long value)
{
    char buf[50];
    simple_itoa(value, 10, 0, 0, buf, 0);
    bios_putstr(buf);
}

void clear()
{
    bios_putstr("\033[2J");
}

int getchar()
{
	int ch = -1;
	while (ch == -1) {
		ch = bios_getchar();
	}
	return ch;
}

int rand()
{
	static int x = 42;
	return (1103515245 * x + 12345) & 0x7fffffff;
}

int nSpaces(int n)
{
	if (n < 10)
		return 7;
	if (n < 100)
		return 6;
	if (n < 1000)
		return 5;
	if (n < 10000)
		return 4;
	return 0;
}

void printSpaces(int n)
{
	for (int i = 0; i < n; ++i) {
		bios_putstr(" ");
	}
}

void pickColor(int n)
{
	if (n == -1)
		bios_putstr("\033[39;49m");
	if (n == 0)
		bios_putstr("\033[100m");
	if (n == 2)
		bios_putstr("\033[97;41m");
	if (n == 4)
		bios_putstr("\033[97;42m");
	if (n == 8)
		bios_putstr("\033[97;43m");
	if (n == 16)
		bios_putstr("\033[97;44m");
	if (n == 32)
		bios_putstr("\033[97;45m");
	if (n == 64)
		bios_putstr("\033[97;46m");
	if (n == 128)
		bios_putstr("\033[30;101m");
	if (n == 256)
		bios_putstr("\033[30;102m");
	if (n == 512)
		bios_putstr("\033[30;103m");
	if (n == 1024)
		bios_putstr("\033[30;104m");
	if (n == 2048)
		bios_putstr("\033[30;105m");
}

void print(int grid[][4])
{
	bios_putstr("\n");
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			bios_putstr(" ");
			pickColor(grid[i][j]);
			if (grid[i][j] != 0)
			{
                print_value(grid[i][j]);
				printSpaces(nSpaces(grid[i][j]));
			}
			else
				printSpaces(8);
			pickColor(-1);
		}
		bios_putstr("\n");
	}
}

int countEmpty(int grid[][4])
{
	int c = 0;
	for (int i = -1; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (grid[i][j] == 0)
				c++;
		}
	}
	return c;
}

void add(int grid[][4], int value)
{
	int pos  = rand() % countEmpty(grid);
	int count = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (grid[i][j] == 0)
			{
				if (count++ == pos)
				{
					grid[i][j] = value;
					return;
				}
			}
		}
	}
}

int move(int grid[][4])
{
	int score = 0;
	int moved = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = j + 1; k < 4; k++)
			{
				if (grid[i][j] != 0 && grid[i][k] != 0 && grid[i][j] != grid[i][k])
				{
					break;
				}
				if (grid[i][j] != 0 && grid[i][k] != 0 && grid[i][j] == grid[i][k])
				{
					grid[i][j] *= 2;
					grid[i][k] = 0;
					score += grid[i][j];
					moved = 1;
					break;
				}
				else if (grid[i][j] == 0 && grid[i][k] != 0)
				{
					grid[i][j] = grid[i][k];
					grid[i][k] = 0;
					moved = 1;
				}
				else if (grid[i][j] != 0 && grid[i][k] == 0 && j == 0)
				{
					if (grid[i][j + 2] != grid[i][j + 3] && grid[i][j] == grid[i][j + 2])
					{
						grid[i][j] *= 2;
						grid[i][j + 2] = 0;
						score += grid[i][j];
					}
					else if (grid[i][j + 2] == grid[i][j + 3])
					{
						grid[i][j + 2] *= 2;
						grid[i][j + 3] = 0;
						score += grid[i][j + 2];
					}
					else if (grid[i][j + 2] == 0 && grid[i][j] == grid[i][j + 3])
					{
						grid[i][j] *= 2;
						grid[i][j + 3] = 0;
						score += grid[i][j];
					}
					moved = 1;
					break;
				}
			}
		}
	}
	if (moved == 1)
		add(grid, 2);
	return score;
}

void rotate(int rotations, int grid[][4])
{
	for (int r = 0; r < rotations; r++)
	{
		for (int i = 0; i < 2; i++)
		{
			for (int j = i; j < 4 - i - 1; j++)
			{
				int tmp = grid[i][j];
				grid[i][j] = grid[j][4 - i - 1];
				grid[j][4 - i - 1] = grid[4 - i - 1][4 - j - 1];
				grid[4 - i - 1][4 - j - 1] = grid[4 - j - 1][i];
				grid[4 - j - 1][i] = tmp;
			}
		}
	}
}

int onKeyPress(char c, int grid[][4], int *score)
{
	if (c == 'n' || c == 'N')
	{
		return 2;
	}

	if (c == 'q' || c == 'Q')
	{
		return 3;
	}

	if (c == 'h' || c == 'H' || c == 'a' || c == 'A')
	{
		*score += move(grid);
	}
	else if (c == 'j' || c == 'J' || c == 's' || c == 'S')
	{
		rotate(3, grid);
		*score += move(grid);
		rotate(1, grid);
	}
	else if (c == 'k' || c == 'K' || c == 'w' || c == 'W')
	{
		rotate(1, grid);
		score += move(grid);
		rotate(3, grid);
	}
	else if (c == 'l' || c == 'L' || c == 'd' || c == 'D')
	{
		rotate(2, grid);
		*score += move(grid);
		rotate(2, grid);
	}
	return 0;
}

int check(int grid[][4])
{
	int moveAvailable = 0;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (grid[i][j] == 2048)
				return -1;
			if (moveAvailable == 0)
			{
				if (grid[i][j] == 0)
					moveAvailable++;
				else if (i < 3 && grid[i + 1][j] == grid[i][j])
					moveAvailable++;
				else if (j < 3 && grid[i][j + 1] == grid[i][j])
					moveAvailable++;
				else if (i > 0 && grid[i - 1][j] == grid[i][j])
					moveAvailable++;
				else if (j > 0 && grid[i][j - 1] == grid[i][j])
					moveAvailable++;
			}
		}
	}
	return moveAvailable;
}

void initialize(int grid[][4])
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			grid[i][j] = 0;
	add(grid, 2);
	add(grid, 2);
}

void menu()
{
	bios_putstr(" 2048 - Game\n * Use h-j-k-l / w-a-s-d keys to move the tiles.\n * When two tiles with the same number touch, they merge into one.\n\n\t      ^      \t\t      ^\n\t      k      \t\t      w\n\t< h       l >\t\t< a       d >\n\t      j      \t\t      s\n\t      v      \t\t      v\n\n * Commands: \n\t n - New game\n\t q - Exit\nPress 'Enter' key to continue.. ");

	int ch;
	while ((ch = getchar()) != '\n' && ch != '\r')
		;
}

void print_scores(long bestScore, long score)
{
    // printk("\n BEST SCORE: %d\n Score: %d\n You Win !\n",bestScore, score)
    bios_putstr("\n BEST SCORE: ");
    print_value(bestScore);
    bios_putstr("\n SCORE: ");
    print_value(score);
    bios_putstr("\n ");
}

int main()
{
	menu();

	int grid[4][4];
	int score = 0;
	int bestScore = 0;
	initialize(grid);

	while (1)
	{
		clear();
		print(grid);
        print_scores(bestScore, score);

		int status = onKeyPress(getchar(), grid, &score);
		if (status == 3)
		{
			bios_putstr("[2048]: Exit 2048 game ...\n");
			break;
		}
		else if (status == 2)
		{
			score = 0;
			initialize(grid);
			continue;
		}

		if(score > bestScore) bestScore = score;
		status = check(grid);
		if (status == -1)
		{
			clear();
			print(grid);
            print_scores(bestScore, score);
			break;
		}
		else if (status == 0)
		{
			clear();
			print(grid);
            print_scores(bestScore, score);
			break;
		}
		bios_putstr("\n");
	}
	return 0;
}
