﻿/*
 * Copyright © 2018 Martino Pilia <martino.pilia@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef RAYCASTVOLUME_H
#define RAYCASTVOLUME_H
#include <QMatrix4x4>
#include <QOpenGLExtraFunctions>
#include <QVector3D>

#include "mesh.h"

/*!
 * \brief Class for a raycasting volume.
 */
class RayCastVolume : protected QOpenGLExtraFunctions
{
public:
	/*!
	 * \brief Create a two-unit cube mesh as the bounding box for the volume.
	 */
	RayCastVolume();

	/*!
	* \brief Destructor.
	*/
	virtual ~RayCastVolume();

	/*!
	* \brief Sets the lookup table that can be used by the shaders.
	* \param image RGB image that should be used as lookup table
	*/
	void setLUT(QImage image);

	/*!
	* \brief Create a test volume that can be used to display some volume data without loading and processing OCT data first.
	*/
	void generateTestVolume();

	/*!
	* \brief Create a noise texture with the size of the viewport.
	*/
	void createNoise();

	void paint();

	/*!
	 * \brief Get the extent of the volume.
	 * \return A vector holding the extent of the bounding box.
	 *
	 * The extent is normalised such that the longest side of the bounding
	 * box is equal to 1.
	 */
	QVector3D extent();

	/*!
	 * \brief Return the model matrix for the volume.
	 * \param shift Shift the volume by its origin.
	 * \return A matrix in homogeneous coordinates.
	 *
	 * The model matrix scales a two-unit side cube to the
	 * extent of the volume.
	 */
	QMatrix4x4 modelMatrix(bool shift = false);

	/*!
	 * \brief Top planes forming the AABB.
	 * \param shift Shift the volume by its origin.
	 * \return A vector holding the intercept of the top plane for each axis.
	 */
	QVector3D top(bool shift = false);

	/*!
	 * \brief Bottom planes forming the AABB.
	 * \param shift Shift the volume by its origin.
	 * \return A vector holding the intercept of the bottom plane for each axis.
	 */
	QVector3D bottom(bool shift = false);

	/*!
	 * \brief Deletes current OpenGL texture and creates a new one with given dimensions. This needs to be done whenever the dimensions of the volume data change.
	 * \param width Width of new texture. Usually this is the number of samples per A-scan
	 * \param height Height of new texture. Usually this is the number of A-scans per B-scan
	 * \param depth Depth of new texture. Usually this is the number of B-scans per Volume which is the same as B-scans per buffer * buffers per volume.
	 */
	void changeTextureSize(unsigned int width, unsigned int height, unsigned int depth);


	void setStretch(float x, float y, float z){this->spacing.setX(x); this->spacing.setY(y); this->spacing.setZ(z);}
	float getStretchX(){return this->spacing.x();}
	float getStretchY(){return this->spacing.y();}
	float getStretchZ(){return this->spacing.z();}

	GLuint getVolumeTexture(){return this->volumeTexture;}

	void setOrigin(float x, float y, float z){this->origin={x,y,z};}
private:
	GLuint volumeTexture;
    GLuint depthPieceTexture;
	GLuint noiseTexture;
	GLuint lutTexture;
	Mesh cubeVao;
	QVector3D origin;
	QVector3D spacing;
	QVector3D size;
	/*!
	 * \brief Scale factor to model space.
	 *
	 * Scale the bounding box such that the longest side equals 1.
	 */
	float getScaleFactor();
};
#endif //RAYCASTVOLUME_H
