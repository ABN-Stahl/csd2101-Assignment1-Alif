#pragma once
/*!***************************************************************************
 * \file   Model.h
 * \author Alif Bin Norazhar(alif.binnorazhar@digipen.edu)
 * \par    CSD2101
 * \date   2026
 * \brief  Abstract base class defining the lifecycle every renderable shape
 *         must follow.  Concrete subclasses (Triangle, Square, ...) override
 *         InitModel() and Render(); Resize() is optional.
 *****************************************************************************/

 class Model
 {
    public:
    Model() {}
    virtual ~Model() {}

    virtual void InitModel() = 0;
    virtual void Render() = 0;
    virtual void Resize(int w, int h) { (void)w; (void)h; }
    
 };