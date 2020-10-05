#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "demo_menu.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("area.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("area.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Boat") boat = &transform;
		else if (transform.name == "Wolf") wolf = &transform;
        else if (transform.name == "Sheep") sheep = &transform;
        else if (transform.name == "Plant") plant = &transform;
	}
//	if (hip == nullptr) throw std::runtime_error("Hip not found.");
//	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
//	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");
//
//	hip_base_rotation = hip->rotation;
//	upper_leg_base_rotation = upper_leg->rotation;
//	lower_leg_base_rotation = lower_leg->rotation;

    Mode::positions.push_back(false);
    Mode::positions.push_back(false);
    Mode::positions.push_back(false);
    Mode::positions.push_back(false);
    Mode::sell.push_back(false);
    Mode::sell.push_back(false);
    Mode::sell.push_back(false);


	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
//	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
            space.pressed = true;
            return true;
        }
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

    if (wolf&&Mode::positions[0] && wolf->position.x<0) {
        wolf->position.x = -wolf->position.x;
    } else if (wolf&&!Mode::positions[0] && wolf->position.x>0) {
        wolf->position.x = -wolf->position.x;
    }
    if (sheep&&Mode::positions[1] && sheep->position.x<0) {
        sheep->position.x = -sheep->position.x;
    } else if (sheep&&!Mode::positions[1] && sheep->position.x>0) {
        sheep->position.x = -sheep->position.x;
    }
    if (plant&&Mode::positions[2] && plant->position.x<0) {
        plant->position.x = -plant->position.x;
    } else if (plant&&!Mode::positions[2] && plant->position.x>0) {
        plant->position.x = -plant->position.x;
    }
    if (Mode::positions[3] && boat->position.x<0) {
        boat->position.x = -boat->position.x;
    } else if (!Mode::positions[3] && boat->position.x>0) {
        boat->position.x = -boat->position.x;
    }
    if (boat->position.x>0) {
        if ((plant&&Mode::sell[2])||(sheep&&plant&&sheep->position.x<0 && plant->position.x<0)) {
            plant->position.z = -plant->position.z;
            plant=nullptr;
        }
        if ((sheep&&Mode::sell[1])||(sheep&&wolf&&wolf->position.x<0 && sheep->position.x<0)) {
            sheep->position.z = -10;
            sheep=nullptr;
        }
    } else {
        if ((plant&&Mode::sell[2])||(sheep&&plant&&sheep->position.x>0 && plant->position.x>0)) {
            plant->position.z = -10;
            plant=nullptr;
        }
        if ((sheep&&Mode::sell[1])||(sheep&&wolf&&wolf->position.x>0 && sheep->position.x>0)) {
            sheep->position.z = -10;
            sheep=nullptr;
        }
    }
    if (space.pressed) {
        space.pressed = false;
        if (phase==0) {
            phase++;
        }else {
            std::vector< MenuMode::Item > items;
            items.emplace_back("Move one of them to the other side");
            if (wolf&&(boat->position.x<0)==(wolf->position.x<0)){
                items.emplace_back("Wolf");
                items.back().on_select = [](MenuMode::Item const&) {
                    std::vector< bool > c(4, false);
                    c[0] = true;
                    c[3] = true;
                    Mode::switch_to_play(c,Mode::sell);
                };
            }
            if (sheep&&(boat->position.x<0)==(sheep->position.x<0)) {
                items.emplace_back("Lamb");
                items.back().on_select = [](MenuMode::Item const &) {
                    std::vector<bool> c(4, false);
                    c[1] = true;
                    c[3] = true;
                    Mode::switch_to_play(c,Mode::sell);
                };
            }
            if (plant&&(boat->position.x<0)==(plant->position.x<0)) {
                items.emplace_back("Plant");
                items.back().on_select = [](MenuMode::Item const &) {
                    std::vector<bool> c(4, false);
                    c[2] = true;
                    c[3] = true;
                    Mode::switch_to_play(c,Mode::sell);
                };
            }
            items.emplace_back("None");
            items.back().on_select = [](MenuMode::Item const&) {
                std::vector< bool > c(4, false);
                c[3] = true;
                Mode::switch_to_play(c,Mode::sell);
            };
            demo_menu->update_items(items);
        }
        Mode::switch_to_demo();
    }

	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));
        constexpr float H = 0.09f;
        std::string s0 = "plant has been eaten";
        std::string s1 = "sheep has been eaten";
        std::string s2 = "plant has been sold";
        std::string s3 = "sheep has been sold";
        std::string s4 = "wolf has been sold";
        if (!plant)lines.draw_text(Mode::sell[2]?s2:s0,
                                   glm::vec3(-0.4f, 0.9f, 0.0),
                                   glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                                   glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        if (!sheep)lines.draw_text(Mode::sell[1]?s3:s1,
                                    glm::vec3(-0.4f, 0.8f, 0.0),
                                    glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                                    glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        if (!wolf)lines.draw_text(s4,
                                   glm::vec3(-0.4f, 0.7f, 0.0),
                                   glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                                   glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

//glm::vec3 PlayMode::get_leg_tip_position() {
//	//the vertex position here was read from the model in blender:
//	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
//}
