#ifndef _GLYPH_H_
#define _GLYPH_H_

#include "../renderable_2d.h"

class glyph : public renderable_2d
{
public:

	void initialise() override;
	void shutdown() override;
	void draw() override;

private:

};

#endif // _GLYPH_H_