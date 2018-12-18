#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <SDL2/SDL.h>

#define WINDOW_NAME "Tree"

#define DEFAULT_WINDOW_WIDTH  800
#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_FLAGS  0

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static int xresolution[] = { 800, 1920 };
static int yresolution[] = { 600, 1080 };

struct state {
	SDL_Renderer *renderer;
	SDL_Window   *window;

	int window_width;
	int window_height;
	int window_flags;

	float len_scale;
	float branch_angle;

	int quit;

	char **args;
	int    numargs;
} state;

static void
clean_up(void)
{
	if (state.renderer) { SDL_DestroyRenderer(state.renderer); }
	if (state.window) { SDL_DestroyWindow(state.window); }

	SDL_Quit();
}

static void
die(const char *msg, ...)
{
	va_list args;
	
	va_start(args, msg);
	
	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");

	va_end(args);
	
	clean_up();

	exit(EXIT_FAILURE);
}

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s [options]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "OPTIONS:\n");
	
	fprintf(stderr, "\t-r\n");
	fprintf(stderr, "\t\tset resolution of the program\n");
	for (unsigned int i = 0; i < ARRAY_SIZE(xresolution); i++) {
		fprintf(stderr, "\t\t%d: %dx%d\n", i, xresolution[i], yresolution[i]);
	}

	fprintf(stderr, "\t-h\n");
	fprintf(stderr, "\t\tdisplay this help message\n");
	clean_up();

	exit(EXIT_SUCCESS);
}

static void
start_up(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		die("%s", SDL_GetError());
	}

	state.window = SDL_CreateWindow(
		WINDOW_NAME, 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED,
		state.window_width,
		state.window_height,
		state.window_flags
	);

	if (state.window == NULL) {
		die("unable to create SDL window");
	}

	state.renderer = SDL_CreateRenderer(
		state.window, 
		-1, 
		SDL_RENDERER_ACCELERATED
	);

	if (state.renderer == NULL) {
		die("unable to create SDL renderer");
	}
}

static void
events(void)
{
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			state.quit = 1;
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE:
				state.quit = 1;
				break;
			case SDLK_UP:
				state.len_scale += 0.05;
				if (state.len_scale >= 1) { 
					state.len_scale -= 0.05; 
				}
				break;
			case SDLK_DOWN:
				state.len_scale -= 0.05;
				if (state.len_scale <= 0) { 
					state.len_scale += 0.05; 
				}
				break;
			case SDLK_LEFT:
				state.branch_angle += M_PI / 16;
				break;
			case SDLK_RIGHT:
				state.branch_angle -= M_PI / 16;
				break;
			}
		}
	}
}

static void
draw_tree(int x1, int y1, int len, float theta)
{
	int dx = len * cos(theta);
	int dy = len * sin(theta);
	int x2 = x1 + dx;
	int y2 = y1 - dy;

	SDL_RenderDrawLine(state.renderer, x1, y1, x2, y2);

	if ((len *= state.len_scale) > 0) {
		draw_tree(x2, y2, len, theta + state.branch_angle);
		draw_tree(x2, y2, len, theta - state.branch_angle);
	}
}

static void
render(void)
{
	int basex = state.window_width >> 1;
	int basey = state.window_height - 10;

	SDL_RenderClear(state.renderer);
	
	draw_tree(basex, basey, basey * 0.3, M_PI / 2);

	SDL_RenderPresent(state.renderer);
}

static void
loop(void)
{
	while (!state.quit) {
		events();
		render();
	}
}

static void
parse_args(int argc, char **argv)
{
	char *progname = argv[0];
	
	memset(&state, 0, sizeof(struct state));
	state.window_width = DEFAULT_WINDOW_WIDTH;
	state.window_height = DEFAULT_WINDOW_HEIGHT;
	state.window_flags = DEFAULT_WINDOW_FLAGS;
	state.len_scale = 0.67;
	state.branch_angle = M_PI / 8;
	state.args = argv;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-r") == 0) {
				if (++i >= argc) {
					die("-r expects an integer argument");
				}

				unsigned int r = atoi(argv[i]);
				if (r >= ARRAY_SIZE(xresolution)) {
					die("invalid resolution code");
				}

				state.window_width = xresolution[r];
				state.window_height = yresolution[r];
			} else if (strcmp(argv[i], "-f") == 0) {
				state.window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			} else if (strcmp(argv[i], "-h") == 0) {
				usage(progname);
			} else {
				die("unrecognized flag \"%s\"", argv[i]);
			}	
		} else {
			state.args[state.numargs++] = argv[i];
		}
	}
}

int
main(int argc, char *argv[])
{
	parse_args(argc, argv);

	start_up();

	loop();

	clean_up();

	return EXIT_SUCCESS;
}