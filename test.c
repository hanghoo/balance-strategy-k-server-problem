/*Testcode for I06 Homework 3 Fall 2020: four servers on grid graph */
/* compiles with command line  gcc test.c -lX11 -lm */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "node.h"
#include <queue> // for queue container
#include <limits.h>
#include <utility> // for pair
#include <map> // for map

using namespace std;

#define SIZE 35
#define BLOCKPERCENTAGE 40
#define FREE 1
#define BLOCKED 0

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_green, gc_pink, gc_cyan, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_green_values,
  gc_pink_values, gc_cyan_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

int stepx, stepy, startx, starty;
void pink_edge(int sx, int sy, int tx, int ty);
void cyan_edge(int sx, int sy, int tx, int ty);
void yellow_edge(int sx, int sy, int tx, int ty);
void green_edge(int sx, int sy, int tx, int ty);

int breadth_first_search(pair <int,int> src, pair <int,int> dest, map< pair<int,int>, pair<int, int> > &candidate_path);
vector<pair <int, int>> find_path(pair <int, int> src, pair <int, int> dest, map< pair<int,int>, pair<int, int> > candidate_path);

int graph[SIZE][SIZE][4];

int main(int argc, char **argv)
{ //int graph[SIZE][SIZE][4]; 
  int i,j,k;
  int sx,sy,tx,ty; /* start and target point */
  int rand_init_value;
  if( argc > 1 )
    {  sscanf(argv[1], "%d", &rand_init_value);
       srand( rand_init_value );
    }
  /* setup grid */
  for( i=0;i< SIZE; i++ )
    for( j=0; j <SIZE; j++)
      { graph[i][j][0]= FREE; graph[i][j][1] = FREE;
	graph[i][j][2]= FREE; graph[i][j][3] = FREE;
      }
  for( i=0;i< SIZE; i++ )
    { graph[i][0][3] = BLOCKED; graph[i][SIZE-1][1] = BLOCKED;
      graph[0][i][2] = BLOCKED; graph[SIZE-1][i][0] = BLOCKED;
    }
  /* Block random edges */
  for( i=0;i< SIZE; i++ )
    for( j=0; j <SIZE-1; j++)
      { if( rand()%100 < BLOCKPERCENTAGE )
	  {   graph[i][j][1]= BLOCKED; graph[i][j+1][3] = BLOCKED;
	  }
	if( rand()%100 < BLOCKPERCENTAGE )
	  {   graph[j][i][0]= BLOCKED; graph[j+1][i][2] = BLOCKED;
	  }
      }
  /* reconnect isolated points */
  for( i=0;i< SIZE; i++ )
    for( j=0; j <SIZE; j++)
      { if( graph[i][j][0] == BLOCKED && graph[i][j][1] == BLOCKED &&
	    graph[i][j][2] == BLOCKED && graph[i][j][3] == BLOCKED )
	  {  graph[i][j][0] = FREE; graph[i][j][1] = FREE;
	     graph[i][j][2] = FREE; graph[i][j][3] = FREE; }
      }
  /* now block the boundary again */
  for( i=0;i< SIZE; i++ )
    { graph[i][0][3] = BLOCKED; graph[i][SIZE-1][1] = BLOCKED;
      graph[0][i][2] = BLOCKED; graph[SIZE-1][i][0] = BLOCKED;
    }
 


  /* opening display: basic connection to X Server */
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  printf("Width %d, Height %d, Screen Number %d\n", 
           display_width, display_height, screen_num);
  /* creating the window */
  border_width = 10;
  win_x = 0; win_y = 0;
  win_height = (int) (display_height/1.3);
  win_width = win_height; /*square window*/
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );
  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );

  /* what events do we want to receive */
  XSelectInput( display_ptr, win, 
            ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  /* finally: put window on screen */
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);

  /* create graphics context, so that we may draw in this window */
  gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
  XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
  XSetLineAttributes( display_ptr, gc, 4, LineSolid, CapRound, JoinRound);
  /* and some other graphics contexts, to draw in yellow and pink and grey*/
  /* yellow*/
  gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
  XSetLineAttributes(display_ptr, gc_yellow, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "yellow", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color yellow\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
  /* green */
  gc_green = XCreateGC( display_ptr, win, valuemask, &gc_green_values);
  XSetLineAttributes(display_ptr, gc_green, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "Chartreuse", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color green\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_green, tmp_color1.pixel );
  /* pink*/
  gc_pink = XCreateGC( display_ptr, win, valuemask, &gc_pink_values);
  XSetLineAttributes( display_ptr, gc_pink, 3, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "Salmon", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color pink\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_pink, tmp_color1.pixel );
  /* cyan*/
  gc_cyan = XCreateGC( display_ptr, win, valuemask, &gc_cyan_values);
  XSetLineAttributes( display_ptr, gc_cyan, 3, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "cyan", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color cyan\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_cyan, tmp_color1.pixel );
  /* grey */
  gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
  XSetLineAttributes( display_ptr, gc_grey, 5, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "dark grey", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color grey\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );

  /* and now it starts: the event loop */
  while(1)
    { XNextEvent( display_ptr, &report );
      switch( report.type )
      { 
        case ConfigureNotify:
          /* This event happens when the user changes the size of the window*/
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          /* break; this case continues into the next:after a resize, 
             the figure gets pinkrawn */
	case Expose:
          /* (re-)draw the figure. This event happens
             each time some part of the window gets exposed (becomes visible) */
        XClearWindow( display_ptr, win );
        stepx = (int) (win_width/(SIZE+3));
        stepy = (int) (win_height/(SIZE+3));
	  	startx = 2*stepx;
	  	starty = 2*stepy;
	  /* Draw Grid Subgraph */
	  for(i = 0; i < SIZE; i++ )
	    for( j = 0; j < SIZE; j++ )
	      { /* First draw edges */
		if(graph[i][j][0] ==  FREE)
		    XDrawLine(display_ptr, win, gc_grey,
			    startx + i*stepx + (int) (0.33*stepx),
			    starty + j*stepy + (int) (0.33*stepy),
                startx + (i+1)*stepx + (int) (0.33*stepx),
			    starty + j*stepy + (int) (0.33*stepy) );
		if(graph[i][j][1] ==  FREE)
		    XDrawLine(display_ptr, win, gc_grey,
			    startx + i*stepx + (int) (0.33*stepx),
			    starty + j*stepy + (int) (0.33*stepy),
                startx + i*stepx + (int) (0.33*stepx),
			    starty + (j+1)*stepy + (int) (0.33*stepy) );
		if(graph[i][j][2] ==  FREE)
		    XDrawLine(display_ptr, win, gc_grey,
			    startx + i*stepx + (int) (0.33*stepx),
			    starty + j*stepy + (int) (0.33*stepy),
                startx + (i-1)*stepx + (int) (0.33*stepx),
			    starty + j*stepy + (int) (0.33*stepy) );
		if(graph[i][j][3] ==  FREE)
		    XDrawLine(display_ptr, win, gc_grey,
			    startx + i*stepx + (int) (0.33*stepx),
			    starty + j*stepy + (int) (0.33*stepy),
                startx + i*stepx + (int) (0.33*stepx),
			    starty + (j-1)*stepy + (int) (0.33*stepy) );
	      }
	  /* now draw vertex */
	  for(i = 0; i < SIZE; i++ )
	    for( j = 0; j < SIZE; j++ )
		XFillArc( display_ptr, win, gc, /*black*/
		       startx+ i*stepx, starty+ j*stepy, /*upper left corner */
		       (int) (0.66*stepx), (int) (0.66*stepy), 0, 360*64);

		/* initial 4 colored corner */
	    XFillArc( display_ptr, win, gc_pink, /*pink*/
		       startx, starty, /* left up corner */
		       (int) (0.66*stepx), (int) (0.66*stepy), 0, 360*64);

	    XFillArc( display_ptr, win, gc_yellow, /*yellow*/
		       startx+(SIZE-1)*stepx, starty, /*right up corner */
		       (int) (0.66*stepx), (int) (0.66*stepy), 0, 360*64);

	    XFillArc( display_ptr, win, gc_cyan, /*cyan*/
		       startx+(SIZE-1)*stepx, starty+(SIZE-1)*stepy, /*right bottom corner */
		       (int) (0.66*stepx), (int) (0.66*stepy), 0, 360*64);
                
        XFillArc( display_ptr, win, gc_green, /*green*/
		       startx, starty+(SIZE-1)*stepy, /*left bottom corner */
		       (int) (0.66*stepx), (int) (0.66*stepy), 0, 360*64);
                
                

          break;

        case ButtonPress:
          /* This event happens when the user pushes a mouse button. */
          {  
	     int x, y; int request_i, request_j; // request_i means x; request_j means y
	     request_i = request_j = -1; /* request position unknown */
  	     x = report.xbutton.x;
         y = report.xbutton.y;

         /* judge the mouseclick point belongs which exactly coordinates */
	     for(i = 0; i < SIZE; i++ )
	        for( j = 0; j < SIZE; j++ )
		  { if( abs( x - (startx + i*stepx + (int) (0.33*stepx)) )
			+ abs(y - ( starty + j*stepy + (int) (0.33*stepy)) )
			< 0.5*stepx )
		      { request_i = i; request_j = j;}
		  }
	     if( request_i >= 0 && request_j >= 0 )
	      {  /* next server request is at (i,j) */ ;
	    /* check graph[x][y][]. FREE=1; BLOCKED=0 */
	     	printf("Coordinate is: (%d,%d)\n", request_i, request_j);
	     	printf("Check right point 0 direction: %d\n", graph[33][34][0]);
	     	printf("Check right point 1 direction: %d\n", graph[33][34][1]);
	     	printf("Check right point 2 direction: %d\n", graph[33][34][2]);
	     	printf("Check right point 3 direction: %d\n", graph[33][34][3]);
	     	printf("\n");

	     	printf("Check right point 0 direction: %d\n", graph[34][34][0]);
	     	printf("Check right point 1 direction: %d\n", graph[34][34][1]);
	     	printf("Check right point 2 direction: %d\n", graph[34][34][2]);
	     	printf("Check right point 3 direction: %d\n", graph[34][34][3]);
	     	printf("\n");

	     	printf("Check right point 0 direction: %d\n", graph[33][33][0]);
	     	printf("Check right point 1 direction: %d\n", graph[33][33][1]);
	     	printf("Check right point 2 direction: %d\n", graph[33][33][2]);
	     	printf("Check right point 3 direction: %d\n", graph[33][33][3]);
	     	printf("\n");

	     	printf("Check right point 0 direction: %d\n", graph[32][34][0]);
	     	printf("Check right point 1 direction: %d\n", graph[32][34][1]);
	     	printf("Check right point 2 direction: %d\n", graph[32][34][2]);
	     	printf("Check right point 3 direction: %d\n", graph[32][34][3]);
	     	printf("\n");

	    /* test the distance calculation by BFS */
	     	pair <int, int> src(0,0);
	     	pair <int, int> dest(request_i,request_j);
	     	map< pair<int,int>, pair<int, int> > candidate_path;
	     	// BFS calculate distance
	     	int distance = breadth_first_search(src, dest, candidate_path);
	     	printf("Distance between pink dot is: %d\n", distance);
	     	printf("Map size is: %d\n", candidate_path.size());
	     	// find every node from src to dest
	     	vector<pair <int, int>> path;
	     	//path = find_path(src, dest, candidate_path);
	     	//printf("Size of path is: %d\n", path.size());

	    /* update colored corner */
	    // XFillArc( display_ptr, win, gc, /*black*/
		   //     startx, starty, upper left corner 
		   //     (int) (0.66*stepx), (int) (0.66*stepy), 0, 360*64);

		/* just to test the colored edges */
		if( graph[request_i][request_j][0] == FREE )
		  pink_edge(request_i, request_j, request_i+1, request_j);
		if( graph[request_i][request_j][1] == FREE )
		  yellow_edge(request_i, request_j, request_i, request_j+1);
		if( graph[request_i][request_j][2] == FREE )
		  cyan_edge(request_i, request_j, request_i-1, request_j);
		if( graph[request_i][request_j][3] == FREE )
		  green_edge(request_i, request_j, request_i, request_j-1);
	      }

          }
          break;
  
         default:
	  /* this is a catch-all for other events; it does not do anything.
             One could look at the report type to see what the event was */ 
          break;
	}

    }
  exit(0);
}

/* draw one unit length. So need record every step */
void yellow_edge(int sx, int sy, int tx, int ty)
{ if( abs(sx-tx) + abs(sy-ty) != 1)
    printf(" called yellow_edge to connect non-neighbors (%d,%d) and (%d,%d)\n", sx,sy, tx, ty);
  else
    XDrawLine(display_ptr, win, gc_yellow,
	      startx + sx*stepx + (int) (0.33*stepx),
	      starty + sy*stepy + (int) (0.33*stepy),
          startx + tx*stepx + (int) (0.33*stepx),
	      starty + ty*stepy + (int) (0.33*stepy) );
}


void green_edge(int sx, int sy, int tx, int ty)
{ if( abs(sx-tx) + abs(sy-ty) != 1)
    printf(" called green_edge to connect non-neighbors (%d,%d) and (%d,%d)\n", sx,sy, tx, ty);
  else
    XDrawLine(display_ptr, win, gc_green,
	      startx + sx*stepx + (int) (0.33*stepx),
	      starty + sy*stepy + (int) (0.33*stepy),
          startx + tx*stepx + (int) (0.33*stepx),
	      starty + ty*stepy + (int) (0.33*stepy) );
}
void pink_edge(int sx, int sy, int tx, int ty)
{ if( abs(sx-tx) + abs(sy-ty) != 1)
    printf(" called pink_edge to connect non-neighbors (%d,%d) and (%d,%d)\n", sx,sy, tx, ty);
  else
    XDrawLine(display_ptr, win, gc_pink,
	      startx + sx*stepx + (int) (0.33*stepx),
	      starty + sy*stepy + (int) (0.33*stepy),
          startx + tx*stepx + (int) (0.33*stepx),
	      starty + ty*stepy + (int) (0.33*stepy) );
}
void cyan_edge(int sx, int sy, int tx, int ty)
{ if( abs(sx-tx) + abs(sy-ty) != 1)
    printf(" called cyan_edge to connect non-neighbors (%d,%d) and (%d,%d)\n", sx,sy, tx, ty);
  else
    XDrawLine(display_ptr, win, gc_cyan,
	      startx + sx*stepx + (int) (0.33*stepx),
	      starty + sy*stepy + (int) (0.33*stepy),
          startx + tx*stepx + (int) (0.33*stepx),
	      starty + ty*stepy + (int) (0.33*stepy) );
}


/* breadth first search to get the distance */
int breadth_first_search(pair <int,int> src, pair <int,int> dest, map< pair<int,int>, pair<int, int> > &candidate_path){
	queue<pair <int,int>> que;
	pair <int,int> cur;
	pair <int,int> tmp;
	//map< pair<int,int>, pair<int, int> > candidate_path;
	int visited[SIZE][SIZE] = {0}; // record whether visited
	que.push(src); // src queue
	visited[src.first][src.second] = 1; // visited

	int n;
	int step = 0;


	while(!que.empty()){
		n = que.size();
		//printf("Queue size is: %d\n", n);
		for(int i=0; i<n; i++){
			cur = que.front(); // get the 1st node
			que.pop(); // pop this node
			if(cur.first  == dest.first  && cur.second == dest.second){
				printf("I find the destination\n");
				printf("(%d, %d)\n", cur.first, cur.second);
				return step; // end criteria
			}
			//printf("Coordinates of node is: (%d, %d)\n", cur.x, cur.y);

			// right
			if(cur.first < SIZE - 1 && graph[cur.first][cur.second][0] == FREE && visited[cur.first + 1][cur.second] == 0){
				tmp.first = cur.first + 1;
				tmp.second = cur.second;
				candidate_path[tmp] = cur;
				que.push(tmp);
				visited[cur.first + 1][cur.second] = 1;
				//printf("%d,%d\n", tmp.x, tmp.y);
				// }			
			}
			// down
			if(cur.second < SIZE - 1 && graph[cur.first][cur.second][1] == FREE && visited[cur.first][cur.second + 1] == 0){
				tmp.first = cur.first;
				tmp.second = cur.second + 1;
				candidate_path[tmp] = cur;
				que.push(tmp);
				visited[cur.first][cur.second + 1] = 1;
			}
			// left
			if(cur.first > 0 && graph[cur.first][cur.second][2] == FREE && visited[cur.first - 1][cur.second] == 0){
				tmp.first = cur.second - 1;
				tmp.first = cur.second;
				candidate_path[tmp] = cur;
				que.push(tmp);
				visited[cur.first - 1][cur.second] = 1;
			}
			// top
			if(cur.second > 0 && graph[cur.first][cur.second][3] == FREE && visited[cur.first][cur.second - 1] == 0){
				tmp.first = cur.first;
				tmp.second = cur.second - 1;
				candidate_path[tmp] = cur;
				que.push(tmp);
				visited[cur.first][cur.second - 1] = 1;
			}
		}

		step++;
	}

	return 0; // queue is empty, the dest cannot reach

}
/* breadth first search to get the distance end */


/* recursive function to find the path */
vector<pair <int, int>> find_path(pair <int, int> src, pair <int, int> dest, map< pair<int,int>, pair<int, int> > candidate_path){
	vector<pair <int, int>> path;
	if(candidate_path.size() == 0){ // src is dest
		return path;
	}

	if(dest.first == src.first && dest.second == src.second){
		return path;
	}
	
	pair<int, int> cur;
	map< pair <int, int>, pair <int, int> >::iterator iter;
    iter = candidate_path.find(dest);
	if(iter != candidate_path.end())
	{
		path.push_back(dest);
	    cur = candidate_path[dest];
	    printf("temp coordinates: (%d, %d)\n", cur.first, cur.second);
	}
	else
	{
	    printf("This point cannot reach\n");
	    return path;
	}
		
	find_path(src, cur, candidate_path);

}
/* recursive function to find the path end */


/* balance strategy */
// int balance_stratege(Server s1, Server s2, Server s3, Server s4, Node dest){
// 	int dist_1, dist_2, dist_3, dist_4;
// 	int dd_1, dd_2, dd_3, dd_4;
// 	dist_1 = breadth_first_search(s1.node, dest);
// 	dist_2 = breadth_first_search(s2.node, dest);
// 	dist_3 = breadth_first_search(s3.node, dest);
// 	dist_4 = breadth_first_search(s4.node, dest);

// 	dd_1 = max(dist_1, s2.distance, s3.distance, s4.distance) - min(dist_1, s2.distance, s3.distance, s4.distance);
// 	dd_2 = max(s1.distance, dist_2, s3.distance, s4.distance) - min(s1.distance, dist_2, s3.distance, s4.distance);
// 	dd_3 = max(s1.distance, s2.distance, dist_3, s4.distance) - min(s1.distance, s2.distance, dist_3, s4.distance);
// 	dd_4 = max(s1.distance, s2.distance, s3.distance, dist_4) - min(s1.distance, s2.distance, s3.distance, dist_4);

// 	int min;
// 	min = min(dd_1, dd_2, dd_3, dd_4);

// 	if(min == dd_1){
// 		return 1;
// 	}
// 	else if(min == dd_2){
// 		return 2;
// 	}
// 	else if(min = dd_3){
// 		return 3;
// 	}
// 	else{
// 		return 4;
// 	}
// }
/* balance strategy end */