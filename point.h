#ifndef SB_POINT_H
#define SB_POINT_H

#define RATIO 1.0 / 3.0

typedef struct Point {
  double x;
  double y;
} Point;

Point *point_create(double x, double y);
void point_free(Point *p);

Point point_add(Point p1, Point p2);
Point point_subtruct(Point p1, Point p2);
Point point_multiply(Point p, double scalar);
Point point_normalize(Point p);
// euclidian length
double point_length(Point p);

void create_handle_triple(Point *p1, Point *p2, Point *p3, Point *h1, Point *h2);
void create_handle_quad(Point *p1, Point *p2, Point *p3, Point *p4, Point *h1, Point *h2);

#endif // SB_POINT_H
