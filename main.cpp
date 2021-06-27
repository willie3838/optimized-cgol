#include <SDL2/SDL.h>
#include <iostream>

#define OFF_COLOUR 0x00
#define ON_COLOUR 0xFF

using namespace std;

unsigned int CELLMAP_WIDTH = 500;
unsigned int CELLMAP_HEIGHT = 500;

unsigned int CELL_SIZE = 1;

unsigned int SCREEN_WIDTH = CELLMAP_WIDTH * CELL_SIZE;
unsigned int SCREEN_HEIGHT = CELLMAP_HEIGHT * CELL_SIZE;

SDL_Window *window = NULL;
SDL_Surface* surface = NULL;

unsigned int seed;

class CellMap
{
    public:
        CellMap(unsigned int w, unsigned int h);
        ~CellMap();
        void SetCell(unsigned int x, unsigned int y);
        void ClearCell(unsigned int x, unsigned int y);
        int CellState(unsigned int x, unsigned int y); 
        void NextGen();
        void Init();
    private:
        unsigned char* cells;
        unsigned char* temp_cells;
        unsigned int width;
        unsigned int height;
        unsigned int length;
};


CellMap::CellMap(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
	length = w * h;
	cells = new unsigned char[length];  
	temp_cells = new unsigned char[length]; 
	memset(cells, 0, length);
}

CellMap::~CellMap()
{
	delete[] cells;
	delete[] temp_cells;
}

void CellMap::SetCell(unsigned int x, unsigned int y)
{
	int xleft, xright, yabove, ybelow;
	unsigned char *cell_ptr = cells + (y * width) + x;

	// Calculate the offsets to the eight neighboring cells,
	// accounting for wrapping around at the edges of the cell map
	xleft = (x == 0) ? width - 1 : -1;
	xright = (x == (width - 1)) ? -(width - 1) : 1;
	yabove = (y == 0) ? length - width : -width;
	ybelow = (y == (height - 1)) ? -(length - width) : width;
    
	*(cell_ptr) |= 0x01; 

    *(cell_ptr + yabove + xleft) += 0x02;
    *(cell_ptr + yabove) += 0x02;
    *(cell_ptr + yabove + xright) += 0x02;
    *(cell_ptr + xleft) += 0x02;
    *(cell_ptr + xright) += 0x02; 
    *(cell_ptr + ybelow + xleft) += 0x02;
    *(cell_ptr + ybelow) += 0x02;
    *(cell_ptr + ybelow + xright) += 0x02;	
}

void CellMap::ClearCell(unsigned int x, unsigned int y)
{
	int xleft, xright, yabove, ybelow;
	unsigned char *cell_ptr = cells + (y * width) + x;

	// Calculate the offsets to the eight neighboring cells,
	// accounting for wrapping around at the edges of the cell map
	xleft = (x == 0) ? width - 1 : -1;
	xright = (x == (width - 1)) ? -(width - 1) : 1;
	yabove = (y == 0) ? length - width : -width;
	ybelow = (y == (height - 1)) ? -(length - width) : width;

	*(cell_ptr) &= ~0x01; // Set first bit to 0

    // Change successive bits for neighbour counts
    *(cell_ptr + yabove + xleft) -= 0x02;
    *(cell_ptr + yabove) -= 0x02;
    *(cell_ptr + yabove + xright) -= 0x02;
    *(cell_ptr + xleft) -= 0x02;
    *(cell_ptr + xright) -= 0x02;
    *(cell_ptr + ybelow + xleft) -= 0x02;
    *(cell_ptr + ybelow) -= 0x02;
    *(cell_ptr + ybelow + xright) -= 0x02;
}

int CellMap::CellState(unsigned int x, unsigned int y)
{
	unsigned char *cell_ptr = cells + (y * width) + x;
	return *cell_ptr & 0x01;
}

void DrawCell(unsigned int x, unsigned int y, unsigned int color )
{
	Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * CELL_SIZE * SCREEN_WIDTH + x * CELL_SIZE) * 4;

	for (unsigned int i = 0; i < CELL_SIZE; i++)
	{
		for (unsigned int j = 0; j < CELL_SIZE; j++)
		{
            *(pixel_ptr + j * 4) = color;
            *(pixel_ptr + j * 4 + 1) = color;
            *(pixel_ptr + j * 4 + 2) = color;
		}
		pixel_ptr += SCREEN_WIDTH * 4;
	}
}

void CellMap::NextGen()
{
    string color;
	unsigned int x, y, count, majColor;
	unsigned char *cell_ptr;

	// Copy to temp map to keep an unaltered version
	memcpy(temp_cells, cells, length);

	// Process all cells in the current cell map
	cell_ptr = temp_cells;
	for (y = 0; y < height; y++) {

		x = 0;
		do {
			// Zero bytes are off and have no neighbours so skip them...
			while (*cell_ptr == 0) {
				cell_ptr++; // Advance to the next cell
				// If all cells in row are off with no neighbours go to next row
				if (++x >= width) goto RowDone;
			}

			// Remaining cells are either on or have neighbours
			count = (*cell_ptr & 0x1F) >> 1; // # of neighboring on-cells
            majColor = (*cell_ptr) >> 6;
            
			if (*cell_ptr & 0x01) {
				// On cell must turn off if not 2 or 3 neighbours
				if ((count != 2) && (count != 3)) {
					ClearCell(x, y);
					DrawCell(x, y, OFF_COLOUR);
				}
			}
			else {
				// Off cell must turn on if 3 neighbours
				if (count == 3) {
					SetCell(x, y);
					DrawCell(x, y, ON_COLOUR);
				}
			}
			// Advance to the next cell byte
			cell_ptr++;

		} while (++x < width);
	RowDone:;
	}
}

void CellMap::Init()
{
	unsigned int x, y, z, init_length;

	// Get seed; random if 0
	seed = (unsigned)time(NULL);

	// Randomly initialise cell map with ~50% on pixels
	cout << "Initializing" << endl;

	srand(seed);
	init_length = (width * height) / 2;
	do
	{
		x = rand() % (width - 1);
		y = rand() % (height - 1);
        z = rand() % 2;

		if(CellState(x, y) == 0){
            if(z == 1){
			    SetCell(x, y);
            }
            else{
                SetCell(x, y);
            }
        }
	} while (--init_length);
}

int main(int argc, char* argv[])
{
	// SDL boilerplate
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	surface = SDL_GetWindowSurface(window);

	// Generation counter
	unsigned long generation = 0;

	// Initialise cell map
	CellMap current_map(CELLMAP_WIDTH, CELLMAP_HEIGHT);
	current_map.Init(); // Randomly initialize cell map

	// SDL Event handler
	SDL_Event e;

	bool quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0) 
			if (e.type == SDL_QUIT) quit = true;

		generation++;

		// Recalculate and draw next generation
		current_map.NextGen();
		// Update frame buffer
		SDL_UpdateWindowSurface(window);
	}

	// Destroy window 
	SDL_DestroyWindow(window); 
	// Quit SDL subsystems 
	SDL_Quit();

	cout << "Total Generations: " << generation
		<< "\nSeed: " << seed << endl;

	return 0;
}