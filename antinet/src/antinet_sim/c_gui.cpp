#include "c_gui.hpp"


std::ostream & operator<<(std::ostream &ostr, t_point3gui obj) {
	return ostr << "(" << obj.x << "," << obj.y << "," << obj.z << ")";
}

c_gui::c_gui() :
	m_selected_object(),
	m_target_ok(false),
	m_source_ok(false),
	m_target_node(),
	m_source_node(),
	camera_x(0),
	camera_y(0),
	camera_z(20),
	camera_zoom(1.0)
{
	move_to_home();
}

void c_gui::move_to_home() {
	camera_x=0;
	camera_y=0;
	camera_z=20;
	camera_zoom=1.0;
}

int c_gui::view_x(int x) const {
	return (int)(x*camera_zoom) - camera_x;
}

int c_gui::view_y(int y) const {
	return (int)(y*camera_zoom) - camera_y;
}


int c_gui::view_x_rev(int x) const {
	return (int)(x+camera_x)/camera_zoom;
}

int c_gui::view_y_rev(int y) const {
	return (int)(y+camera_y)/camera_zoom;
}


int c_gui::view_x() const { return view_x(m_cursor.x); }
int c_gui::view_y() const { return view_y(m_cursor.y); }
int c_gui::view_x_rev() const { return view_x_rev(m_cursor.x); }
int c_gui::view_y_rev() const { return view_x_rev(m_cursor.y); }


