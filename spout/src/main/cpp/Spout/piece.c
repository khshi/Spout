#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "piece.h"
#include "spout.h"
#include "font.h"

#ifndef ANDROID_NDK
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <GLES/gl.h>
#include <GLES/glext.h>

#include "../SpoutNativeLibProxy.h" // For Keys
#endif // ANDROID_NDK

/**********GLOBALS***********************/

// RGB to RGB565 macro
#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))

int colorTable[19];

// Colors
int color_on = 0;

#ifndef ANDROID_NDK
SDL_Surface *video;

static GLfloat texcoord[4];

int x_off = 20;
int y_off = 20;
int fullscreen = 0;
#else
// Useful macros
#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 128
#define S_PIXELS_SIZE (sizeof(texture_map[0]) * TEXTURE_WIDTH * TEXTURE_HEIGHT)
#define KEY_UNPRESSED 0
#define KEY_PRESSED 1

static uint16_t *texture_map = 0;

// Texture offset
static int s_x = 0;
static int s_y = 0;

// Display offset
int dis_x = 25;
int dis_y = 25;

// Vibration
int vibrate_now = 0;

// Sound
int sound_on = 0;

// Long tail
int tail_on = 0;

// Scores
int score_height = 0;
int score_score = 0;

// Filter
int filter = GL_NEAREST;

// Cube settings
int cube_on = 0;
GLfloat angleCube = 0.0f;
GLfloat speedCube = -0.5f;
float vertices[] = { // Vertices for a face
	-1.0f, -1.0f, 0.0f,  // 0. left-bottom-front
	1.0f, -1.0f, 0.0f,  // 1. right-bottom-front
	-1.0f,  1.0f, 0.0f,  // 2. left-top-front
	1.0f,  1.0f, 0.0f   // 3. right-top-front
};

// Scales multipliers
const GLfloat x_scale_m = (float)IN_SCREEN_WIDTH / TEXTURE_WIDTH;
const GLfloat y_scale_m = (float)IN_SCREEN_HEIGHT / TEXTURE_HEIGHT;

// Texture offsets
float texCoords[8];

// Useless features
static GLuint s_disable_caps[] = {
	GL_FOG,
	GL_LIGHTING,
	GL_CULL_FACE,
	GL_ALPHA_TEST,
	GL_BLEND,
	GL_COLOR_LOGIC_OP,
	GL_DITHER,
	GL_STENCIL_TEST,
	GL_DEPTH_TEST,
	GL_COLOR_MATERIAL,
	0
};

void initCubeTexturesCoords();
#endif // !ANDROID_NDK

static GLuint global_texture = 0;

unsigned char *vBuffer = NULL;
unsigned char pixelData[IN_SCREEN_WIDTH * IN_SCREEN_HEIGHT];
unsigned short pixelDataRGB565[IN_SCREEN_WIDTH * IN_SCREEN_HEIGHT];

// 128 256 384 512
// 88 176 264 352

int display_w = 640;
int display_h = 480;

int interval = 0;
int exec = 1;

unsigned char *keys;

int font_posX = 0, font_posY = 0, font_width = 4, font_height = 6;
unsigned char font_fgcolor = 3, font_bgcolor = 0, font_bgclear = 0;
const char *font_adr = (const char *)FONT6;

/**********FUNCTIONS*********************/

void pceLCDDispStop () { }

void pceLCDDispStart () { }

void initilizeColorTable() {
    colorTable[0] = RGB565(139, 0, 0);      // DarkRed
    colorTable[1] = RGB565(255, 160, 122);  // LightSalmon
    colorTable[2] = RGB565(220, 20, 60);    // Crimson
    colorTable[3] = RGB565(178, 34, 34);    // FireBrick
    colorTable[4] = RGB565(255, 0, 0);      // Red
    colorTable[5] = RGB565(255, 69, 0);     // OrangeRed
    colorTable[6] = RGB565(255, 140, 0);    // DarkOrange
    colorTable[7] = RGB565(255, 165, 0);    // Orange
    colorTable[8] = RGB565(255, 255, 0);    // Yellow
    colorTable[9] = RGB565(255, 215, 0);    // Gold
    colorTable[10] = RGB565(128, 0, 0);     // Maroon
    colorTable[11] = RGB565(210, 105, 30);  // Chocolate
    colorTable[12] = RGB565(135, 206, 250); // LightSkyBlue
    colorTable[13] = RGB565(30, 144, 255);  // DodgerBlue
    colorTable[14] = RGB565(0, 255, 0);     // Green

    /***********************************/

    colorTable[15] = 0xAD55;                // LightGray #1
    colorTable[16] = 0x52AA;                // DarkGray #2
    colorTable[17] = 0x0000;                // Black
    colorTable[18] = 0xFFFF;                // White
}

#ifndef ANDROID_NDK
int rgb (unsigned char r, unsigned char g, unsigned char b) {
    unsigned char red = r >> 3;
    unsigned char green = g >> 2;
    unsigned char blue = b >> 3;

    int result = (red << (5 + 6)) | (green << 5) | blue;

    printf("red: %x\n", red);
    printf("green: %x\n", green);
    printf("blue: %x\n", blue);
    printf("result: %x\n", result);

    return result;
}

void SDL_GL_Enter2DMode () {
	SDL_Surface *screen = SDL_GetVideoSurface();

	/* Note, there may be other things you need to change,
	   depending on how you have your OpenGL state set up.
	*/
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	/* This allows alpha blending of 2D textures with the scene */
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glViewport(0, 0, screen->w, screen->h);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0, (GLdouble)screen->w, (GLdouble)screen->h, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void SDL_GL_Leave2DMode () {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

GLuint SDL_GL_LoadTexture_fromPixelData (int w, int h, GLfloat *texcoord, void *pixels) {
    GLuint texture;

    texcoord[0] = 0.0f;         /* Min X */
    texcoord[1] = 0.0f;         /* Min Y */
    texcoord[2] = 1.0f;         /* Max X */
    texcoord[3] = 1.0f;         /* Max Y */

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    return texture;
}

void initSDL () {
    if (SDL_Init (SDL_INIT_VIDEO) < 0) {
        fprintf (stderr, "Couldn't initialize SDL: %s\n", SDL_GetError ());
        exit (1);
    }

    atexit (SDL_Quit);

    Uint32 flags = SDL_OPENGL | SDL_SWSURFACE;
    if (fullscreen) {
        flags |= SDL_FULLSCREEN;
    }

    video = SDL_SetVideoMode (display_w, display_h, 16, flags);
    if (video == NULL) {
        fprintf (stderr, "Couldn't set video mode: %s\n", SDL_GetError ());
        exit (1);
    }

    /* OpenGL Init */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 3 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 3 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 2 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0 ); // ?

    glClearColor( 255.0, 255.0, 255.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_ShowCursor (0);

    SDL_WM_SetCaption("Spout", NULL);

    if (!global_texture) {
        global_texture = SDL_GL_LoadTexture_fromPixelData(IN_SCREEN_WIDTH, IN_SCREEN_HEIGHT, texcoord, video->pixels);
    }

    initilizeColorTable();
}
#else
void initSpoutGLES() {

	keys = keysState;

	initilizeColorTable();

	if (cube_on == 1) {
		initCubeTexturesCoords();
	}

	pceAppInit ();

	texture_map = malloc(S_PIXELS_SIZE);

	resizeSpoutGLES(display_w, display_h);
}

static void render_pixels(uint16_t *pixels, uint16_t *from_pixels)
{
	int x, y;
	for (y = s_y; y < s_y + IN_SCREEN_HEIGHT; y++) {
		for (x = s_x; x < s_x + IN_SCREEN_WIDTH; x++) {
			int idx = x + y * TEXTURE_WIDTH;
			int ry = y - s_y;
			int rx = x - s_x;
			int idx2 = rx + ry * IN_SCREEN_WIDTH;
			pixels[idx++] = from_pixels[idx2];
		}
	}
}

static void check_gl_error(const char* op)
{
	GLint error;
	for (error = glGetError(); error; error = glGetError()) {
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}

void deinitSpoutGLES() {

	// pceAppExit ();

	// GLSurfaceView does its own EGL context management.
	// It takes all responsibility for creating it, destroying it,
	// and ensuring that it's current on the renderer thread
	// when onDrawFrame() is called. If this is not what you want,
	// you should use a plain SurfaceView instead, and issue the
	// various EGL calls yourself.
	// http://stackoverflow.com/a/22897020

	// There's really no need.
	// When the GLSurfaceView gets destroyed, it will free its EGL context,
	// which will release all resources associated with it.
	// http://stackoverflow.com/a/12172410

//	if (global_texture) {
//		glDeleteTextures(1, &global_texture);
//		global_texture = 0;
//	}

	if (texture_map) {
		LOGI("Free texture_map...");
		free(texture_map);
	}

	exit(0);
}

void initCubeTexturesCoords() {
	// A. left-bottom
	texCoords[0] = 0.0f;
	texCoords[1] = y_scale_m;

	// B. right-bottom
	texCoords[2] = x_scale_m;
	texCoords[3] = y_scale_m;

	// C. left-top
	texCoords[4] = 0.0f;
	texCoords[5] = 0.0f;

	// D. right-top
	texCoords[6] = x_scale_m;
	texCoords[7] = 0.0f;
}

void emulateGLUperspective(GLfloat fovY, GLfloat aspect,
		GLfloat zNear,  GLfloat zFar)
{
	GLfloat fW, fH;

	fH = tan(fovY / 180 * M_PI) * zNear / 2;

	fW = fH * aspect;

	glFrustumf(-fW, fW, -fH, fH, zNear, zFar);
}

void resizeSpoutGLES(int w, int h) {

	LOGI("Resize Spout GLES: %d, %d", w, h);

	glDeleteTextures(1, &global_texture);
	GLuint *start = s_disable_caps;
	while (*start) {
		glDisable(*start++);
	}

	if (cube_on == 1) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glShadeModel(GL_SMOOTH);
	}

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &global_texture);
	glBindTexture(GL_TEXTURE_2D, global_texture);

	glTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, filter);
	glTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, filter);

	glShadeModel(GL_FLAT);
	check_gl_error("glShadeModel");

	glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
	check_gl_error("glColor4x");

	int rect[4] = { 0, IN_SCREEN_HEIGHT, IN_SCREEN_WIDTH, -IN_SCREEN_HEIGHT };

	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
	check_gl_error("glTexParameteriv");

	glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGB,
			TEXTURE_WIDTH,
			TEXTURE_HEIGHT,
			0,
			GL_RGB,
			GL_UNSIGNED_SHORT_5_6_5,
			NULL);
	check_gl_error("glTexImage2D");

	if (cube_on == 1) {
		GLclampf gray = 96.0f / 255.0f;
		glClearColor(gray, gray, gray, 0.0f);
	} else {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
	check_gl_error("glClearColor");

	display_w = w;
	display_h = h;

	if (cube_on == 1) {
		// To prevent division by 0
		if (display_h == 0) {
			display_h = 1;
		}

		float aspect = (float)display_w / display_h;

		glViewport(0, 0, display_w, display_h);

		// Setup perspective projection, with aspect ratio matches viewport
		glMatrixMode(GL_PROJECTION);  // Select projection matrix
		glLoadIdentity();             // Reset projection matrix

		// Use perspective projection
		// GLU library is not present in the Android NDK
		// Emulate this function
		// gluPerspective(gl, 45, aspect, 0.1f, 100.f);
		emulateGLUperspective(45.0f, aspect, 0.1f, 100.0f);

		glMatrixMode(GL_MODELVIEW);   // Select model-view matrix
		glLoadIdentity();             // Reset projection matrix
	}
}

void draw_cube(void) {
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texCoords);

	// front
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	// left
	glPushMatrix();
	glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	// back
	glPushMatrix();
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	// right
	glPushMatrix();
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	// top
	glPushMatrix();
	glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	// bottom
	glPushMatrix();
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glPopMatrix();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_CULL_FACE);
}
#endif // !ANDROID_NDK

void pceLCDTrans () {
    static int w, h;
    int x, y;
    unsigned char *vbi, *bi;
    unsigned char *bline;
    const int zoom = 1;

    const int offsetx = IN_SCREEN_WIDTH/2 - 128*zoom/2;
    const int offsety = IN_SCREEN_HEIGHT/2 - 88*zoom/2;

    bi = pixelData;

    w = IN_SCREEN_WIDTH;
    h = IN_SCREEN_HEIGHT;

    // Read display state from Engine
    for (y = 0; y < (88*zoom); y++) {
        vbi = vBuffer +  (y/zoom) * 128;  //the actual line on the pce internal buffer (128x88)
        bline = bi + IN_SCREEN_WIDTH * (y + offsety);
        bline += offsetx;
        for (x = 0; x < (128*zoom); x++) {
            *bline++ = *(vbi + x/zoom);
        }
    }

    // Convert buffer to RGB565
    int rz;
    for (rz = 0; rz < IN_SCREEN_HEIGHT * IN_SCREEN_WIDTH; ++rz) {
        switch (pixelData[rz]) {
        case 0x1:
            pixelDataRGB565[rz] = (color_on) ? colorTable[rand() % 15] : colorTable[15];
            break;
        case 0x2:
            pixelDataRGB565[rz] = colorTable[16]; // DarkGray #2
            break;
        case 0x3:
            pixelDataRGB565[rz] = colorTable[17]; // Black
            break;
        case 0x0:
            pixelDataRGB565[rz] = colorTable[18]; // White
            break;
        default:
            break;
        }
    }

#ifndef ANDROID_NDK
    glClearColor( 255.0, 255.0, 255.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SDL_GL_Enter2DMode();

    GLfloat texMinX = texcoord[0];
    GLfloat texMinY = texcoord[1];
    GLfloat texMaxX = texcoord[2];
    GLfloat texMaxY = texcoord[3];

    int x_coord = 0;
    int y_coord = 0;

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixelDataRGB565);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(texMinX, texMinY); glVertex2i(x_coord + x_off,             y_coord + y_off );
    glTexCoord2f(texMaxX, texMinY); glVertex2i(x_coord + display_w - x_off, y_coord + y_off );
    glTexCoord2f(texMinX, texMaxY); glVertex2i(x_coord + x_off,             y_coord + display_h - y_off);
    glTexCoord2f(texMaxX, texMaxY); glVertex2i(x_coord + display_w - x_off, y_coord + display_h - y_off);
    glEnd();

    SDL_GL_Leave2DMode();

    SDL_GL_SwapBuffers();
#else
	memset(texture_map, 0, S_PIXELS_SIZE);
	render_pixels(texture_map, pixelDataRGB565);
	glClear(GL_COLOR_BUFFER_BIT);
	glTexSubImage2D(GL_TEXTURE_2D,
			0,
			0,
			0,
			TEXTURE_WIDTH,
			TEXTURE_HEIGHT,
			GL_RGB,
			GL_UNSIGNED_SHORT_5_6_5,
			texture_map);
	check_gl_error("glTexSubImage2D");

	if (cube_on == 0) {
		glDrawTexiOES(dis_x, dis_y, 0,
				display_w - dis_x * 2,
				display_h - dis_y * 2);
		check_gl_error("glDrawTexiOES");
	} else {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -3.5f);
		glRotatef(angleCube, 0.1f, 1.0f, 0.2f);

		draw_cube();

		angleCube += speedCube;
	}
#endif // !ANDROID_NDK
}

int pcePadGet () {
    static int pad = 0;
    int i = 0, op = pad & 0x00ff;

#ifndef ANDROID_NDK
    int k[] = {
        SDLK_PAGEUP, SDLK_PAGEDOWN, SDLK_LEFT, SDLK_RIGHT,
        SDLK_KP4, SDLK_KP6,
    #ifdef TARGET_PANDORA
        SDLK_PAGEUP, SDLK_PAGEDOWN, SDLK_HOME, SDLK_END,
    #else
        SDLK_x, SDLK_z, SDLK_SPACE, SDLK_RETURN,
    #endif
        SDLK_ESCAPE, SDLK_LSHIFT, SDLK_RSHIFT,
        SDLK_PLUS, SDLK_MINUS
    };
#else
    int k[] = {
        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
        KEY_LEFT, KEY_RIGHT,
        KEY_FIRE, KEY_FIRE, KEY_FIRE, KEY_FIRE,
		KEY_PAUSE, KEY_QUIT, KEY_UNKNOWN,
		KEY_FIRE, KEY_FIRE
    };
#endif // !ANDROID_NDK

    int p[] = {
        PAD_UP, PAD_DN, PAD_LF, PAD_RI,
        PAD_LF, PAD_RI,
        PAD_A, PAD_B, PAD_A, PAD_B,
        PAD_C, PAD_D, PAD_D, -1
    };

    pad = 0;

    do {
#ifndef ANDROID_NDK
        if (keys[k[i]] == SDL_PRESSED) {
#else
        if (keys[k[i]] == KEY_PRESSED) {
#endif // !ANDROID_NDK
            pad |= p[i];
        }
        i++;
    } while (p[i] >= 0);

    pad |= (pad & (~op)) << 8;

    return pad;
}

void pceAppSetProcPeriod (int period) {
    interval = period;
}

void pceAppReqExit (/*int c*/) {
    exec = 0;
}

unsigned char * pceLCDSetBuffer (unsigned char *pbuff)
{
    if (pbuff) {
        vBuffer = pbuff;
    }
    return vBuffer;
}

void pceFontSetType (int type)
{
    const int widths[] = { 5, 8, 4};
    const int heights[] = { 10, 16, 6};
    const char *adr[] = { (const char *)FONT6, (const char *)FONT16, (const char *)FONT6 };

    type &= 3;
    font_width = widths[type];
    font_height = heights[type];
    font_adr = adr[type];
}

void pceFontSetTxColor (int color)
{
    font_fgcolor = (unsigned char) color;
}

void pceFontSetBkColor (int color)
{
    if (color >= 0) {
        font_bgcolor = (unsigned char) color;
        font_bgclear = 0;
    } else {
        font_bgclear = 1;
    }
}

void pceFontSetPos (int x, int y)
{
    font_posX = x;
    font_posY = y;
}

void pceFontPrintf (const char *fmt, ...)
{
    unsigned char *adr = vBuffer + font_posX + font_posY * 128;
    unsigned char *pC;
    char c[1024];
    va_list argp;

    va_start (argp, fmt);
    vsprintf (c, fmt, argp);
    va_end (argp);

    pC = (unsigned char*)c;
    while (*pC) {
        int i, x, y;
        const unsigned char *sAdr;
        if (*pC >= 0x20 && *pC < 0x80) {
            i = *pC - 0x20;
        } else {
            i = 0;
        }
        sAdr = (const unsigned char *)font_adr + (i & 15) + (i >> 4) * 16 * 16;
        for (y = 0; y < font_height; y++) {
            unsigned char c = *sAdr;
            for (x = 0; x < font_width; x++) {
                if (c & 0x80) {
                    *adr = font_fgcolor;
                } else if (font_bgclear == 0) {
                    *adr = font_bgcolor;
                }
                adr++;
                c <<= 1;
            }
            adr += 128 - font_width;
            sAdr += 16;
        }
        adr -= 128 * font_height - font_width;
        pC++;
    }
}

#ifndef ANDROID_NDK
int pceFileOpen (FILEACC * pfa, const char *fname, int mode)
{
//    fprintf(stderr, "Openning file: %s\n", fname);

    if (mode == FOMD_RD) {
        *pfa = open (fname, O_RDONLY);
    } else if (mode == FOMD_WR) {
        *pfa = open (fname, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
    }

    if (*pfa >= 0) {
        return 0;
    } else {
        return 1;
    }
}

int pceFileReadSct (FILEACC * pfa, void *ptr, /*int sct,*/ int len)
{
    return read (*pfa, ptr, len);
}

int pceFileWriteSct (FILEACC * pfa, const void *ptr, /*int sct,*/ int len)
{
    return write (*pfa, ptr, len);
}

int pceFileClose (FILEACC * pfa)
{
    close (*pfa);
    return 0;
}
#else
void pceFileReadSct (void *ptr, /*int sct,*/ int len)
{
    int *toScore = (int *)(ptr);
    toScore[0] = score_score;
    toScore[1] = score_height;
}

void pceFileWriteSct (const void *ptr, /*int sct,*/ int len)
{
	const int *hiScore = (const int *)(ptr);
	if (javaEnviron != NULL) {
		// Set high-scores to JAVA-launcher/activity
		jclass clazz = (*javaEnviron)->FindClass(javaEnviron, "ru/exlmoto/spout/SpoutActivity");
		if (clazz == 0) {
			LOGI("Error JNI: Class ru/exlmoto/spout/SpoutActivity not found!");
		}

		jmethodID methodId = (*javaEnviron)->GetStaticMethodID(javaEnviron, clazz, "setScores", "(II)V");
		if (methodId == 0) {
			LOGI("Error JNI: methodId is 0, method setScores (II)V not found!");
		}

		// Call JAVA-method
		(*javaEnviron)->CallStaticVoidMethod(javaEnviron, clazz, methodId, hiScore[1], hiScore[0]);

		(*javaEnviron)->DeleteLocalRef(javaEnviron, clazz);
	}
}
#endif // !ANDROID NDK

#ifndef ANDROID_NDK
int main (/*int argc, char *argv[]*/)
{
    SDL_Event event;
    int nextTick, wait;
    int cnt = 0;

    initSDL ();
    pceAppInit ();

    nextTick = SDL_GetTicks () + interval;
    while (exec) {
        keys = SDL_GetKeyState (NULL);

        while (SDL_PollEvent(&event)) { }

        wait = nextTick - SDL_GetTicks ();
        if (wait > 0) {
            SDL_Delay (wait);
        }

        pceAppProc ();
        pceLCDTrans ();

        nextTick += interval;
        cnt++;

        if ((keys[SDLK_ESCAPE] == SDL_PRESSED && (keys[SDLK_LSHIFT] == SDL_PRESSED || keys[SDLK_RSHIFT] == SDL_PRESSED)) || event.type == SDL_QUIT) {
            exec = 0;
        }
    }

    pceAppExit ();

    if (global_texture) {
        glDeleteTextures(1, &global_texture);
        global_texture = 0;
    }

    return 0;
}
#else
void stepSpoutGLES() {
	pceAppProc();
	pceLCDTrans();
}

int getScoreScores() {
	return score;
}

int getScoreHeight() {
	return height;
}

void vibrateFromJNI(int duration) {
	if (vibrate_now) {
		if (javaEnviron != NULL) {
			jclass clazz = (*javaEnviron)->FindClass(javaEnviron, "ru/exlmoto/spout/SpoutActivity");
			if (clazz == 0) {
				LOGI("Error JNI: Class ru/exlmoto/spout/SpoutActivity not found!");
			}

			jmethodID methodId = (*javaEnviron)->GetStaticMethodID(javaEnviron, clazz, "doVibrate", "(I)V");
			if (methodId == 0) {
				LOGI("Error JNI: methodId is 0, method doVibrate (I)V not found!");
			}

			// Call JAVA-method
			(*javaEnviron)->CallStaticVoidMethod(javaEnviron, clazz, methodId, duration);
			(*javaEnviron)->DeleteLocalRef(javaEnviron, clazz);
		}
	}
}

void playGameOverSoundFromJNI() {
	if (sound_on) {
		if (javaEnviron != NULL) {
			// Get SpoutSounds class from SpoutActivity class
			jclass clazz = (*javaEnviron)->FindClass(javaEnviron, "ru/exlmoto/spout/SpoutActivity$SpoutSounds");
			if (clazz == 0) {
				LOGI("Error JNI: Class ru/exlmoto/spout/SpoutActivity$SpoutSounds not found!");
			}

			// Get static SoundID field ID from static SpoutSounds class
			jfieldID fieldID = (*javaEnviron)->GetStaticFieldID(javaEnviron, clazz, "s_gameover", "I");
			if (fieldID == 0) {
				LOGI("Error JNI: fieldID is 0, field s_gameover I not found!");
			}

			// Get jint value from static class field
			jint soundID = (*javaEnviron)->GetStaticIntField(javaEnviron, clazz, fieldID);
			LOGI("JNI: soundID is: %d", (int)soundID);

			(*javaEnviron)->DeleteLocalRef(javaEnviron, clazz);

			// Now return to SpoutActivity
			clazz = (*javaEnviron)->FindClass(javaEnviron, "ru/exlmoto/spout/SpoutActivity");
			if (clazz == 0) {
				LOGI("Error JNI: Class ru/exlmoto/spout/SpoutActivity not found!");
			}

			// Get static playSound method from SpoutActivity class
			jmethodID methodId = (*javaEnviron)->GetStaticMethodID(javaEnviron, clazz, "playSound", "(I)V");
			if (methodId == 0) {
				LOGI("Error JNI: methodId is 0, method playSound (I)V not found!");
			}

			if (soundID != 0) {
				// Call JAVA-method
				(*javaEnviron)->CallStaticVoidMethod(javaEnviron, clazz, methodId, soundID);
			}

			(*javaEnviron)->DeleteLocalRef(javaEnviron, clazz);
		}
	}
}
#endif // !ANDROID_NDK
