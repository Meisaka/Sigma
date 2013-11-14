#include "components/GLScreenCursor.h"
#include "resources/GLTexture.h"

namespace Sigma {
	namespace event {
		namespace handler {
		GLScreenCursor::GLScreenCursor(int entityID) : GLScreenQuad(entityID), show(true), hotspotx(0.0f), hotspoty(0.0f) {}
		GLScreenCursor::~GLScreenCursor() {}

		void GLScreenCursor::InitializeBuffers() {
			float left = -1.0f;
			float top = 1.0f;
			float right = (this->w) * 2.0f - 1.0f;
			float bottom = (this->h) * -2.0f + 1.0f;
			this->AddVertex(Vertex(left, top, 0.0f));
			this->AddVertex(Vertex(right, top, 0.0f));
			this->AddVertex(Vertex(left, bottom, 0.0f));
			this->AddVertex(Vertex(right, bottom, 0.0f));

			this->AddFace(Face(0, 1, 2));
			this->AddFace(Face(2, 1, 3));

			this->texCoords.push_back(TexCoord(0.0f, 1.0f));
			this->texCoords.push_back(TexCoord(1.0f, 1.0f));
			this->texCoords.push_back(TexCoord(0.0f, 0.0f));
			this->texCoords.push_back(TexCoord(1.0f, 0.0f));

			// Add the mesh group
			this->AddMeshGroupIndex(0);

			glGenSamplers(1, &this->samplerid);
			glSamplerParameteri(this->samplerid, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glSamplerParameteri(this->samplerid, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glSamplerParameteri(this->samplerid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glSamplerParameteri(this->samplerid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			this->shader->Use();
			this->shader->AddUniform("in_Texture");
			this->shader->AddUniform("in_Pos");
			this->shader->AddUniform("in_Hotspot");

			// Rift stuff
			this->shader->AddUniform("in_View");
			this->shader->AddUniform("in_Proj");
			this->shader->AddUniform("enable_projection");

			this->shader->UnUse();

			GLMesh::InitializeBuffers();
		}
		void GLScreenCursor::MouseMove(float x, float y, float dx, float dy) { this->x = x; this->y = y; }
		void GLScreenCursor::MouseDown(Sigma::event::BUTTON btn, float x, float y) {}
		void GLScreenCursor::MouseUp(Sigma::event::BUTTON btn, float x, float y) {}

		void GLScreenCursor::Render(glm::mediump_float *view, glm::mediump_float *proj) {
			if (this->show && this->texture) {
				this->shader->Use();

				glDisable(GL_CULL_FACE);
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);

				glUniformMatrix4fv((*this->shader)("in_View"), 1, GL_FALSE, view);
				glUniformMatrix4fv((*this->shader)("in_Proj"), 1, GL_FALSE, proj);

				glUniform2f((*this->shader)("in_Pos"), this->x * 2.0f, this->y * -2.0f);
				glUniform2f((*this->shader)("in_Hotspot"), this->w * this->hotspotx, this->h * this->hotspoty);
				glBindVertexArray(this->vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->GetBuffer(this->ElemBufIndex));

				glUniform1i((*this->shader)("in_Texture"), 0);
				glBindTexture(GL_TEXTURE_2D, this->texture->GetID());
				glActiveTexture(GL_TEXTURE0);
				glBindSampler(0, this->samplerid);

				for (int i = 0, cur = this->MeshGroup_ElementCount(0), prev = 0; cur != 0; prev = cur, cur = this->MeshGroup_ElementCount(++i)) {
					glDrawElements(this->DrawMode(), cur, GL_UNSIGNED_INT, (void*)prev);
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
				glBindVertexArray(0);
				glBindSampler(0, 0);
				// Clear the texture for next frame

				glBindTexture(GL_TEXTURE_2D, 0);

				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
				glEnable(GL_CULL_FACE);

				this->shader->UnUse();
			}
		}
		};
	};
};