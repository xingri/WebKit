/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSVG_SVGPatternElementImpl_H
#define KSVG_SVGPatternElementImpl_H
#ifdef SVG_SUPPORT

#include "SVGTests.h"
#include "SVGLangSpace.h"
#include "SVGURIReference.h"
#include "SVGFitToViewBox.h"
#include "SVGStyledLocatableElement.h"
#include "SVGExternalResourcesRequired.h"

#include <kcanvas/device/KRenderingPaintServerPattern.h>

class KCanvasImage;

namespace WebCore
{
    class SVGLength;
    class SVGPatternElement;
    class SVGTransformList;
    class SVGPatternElement : public SVGStyledLocatableElement,
                                  public SVGURIReference,
                                  public SVGTests,
                                  public SVGLangSpace,
                                  public SVGExternalResourcesRequired,
                                  public SVGFitToViewBox,
                                  public KCanvasResourceListener
    {
    public:
        SVGPatternElement(const QualifiedName&, Document*);
        virtual ~SVGPatternElement();
        
        virtual bool isValid() const { return SVGTests::isValid(); }

        // 'SVGPatternElement' functions
        virtual void parseMappedAttribute(MappedAttribute *attr);

        const SVGStyledElement *pushAttributeContext(const SVGStyledElement *context);

        virtual void resourceNotification() const;
        virtual void notifyAttributeChange() const;

        virtual bool rendererIsNeeded(RenderStyle *style) { return StyledElement::rendererIsNeeded(style); }
        virtual RenderObject *createRenderer(RenderArena *arena, RenderStyle *style);
        virtual KRenderingPaintServerPattern *canvasResource();

        // 'virtual SVGLocatable' functions
        virtual SVGMatrix *getCTM() const;

    protected:
        ANIMATED_PROPERTY_FORWARD_DECLARATIONS(SVGURIReference, String, Href, href)
        ANIMATED_PROPERTY_FORWARD_DECLARATIONS(SVGExternalResourcesRequired, bool, ExternalResourcesRequired, externalResourcesRequired)
        ANIMATED_PROPERTY_FORWARD_DECLARATIONS(SVGFitToViewBox, FloatRect, ViewBox, viewBox)
        ANIMATED_PROPERTY_FORWARD_DECLARATIONS(SVGFitToViewBox, SVGPreserveAspectRatio*, PreserveAspectRatio, preserveAspectRatio)

        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, SVGLength*, RefPtr<SVGLength>, X, x)
        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, SVGLength*, RefPtr<SVGLength>, Y, y)
        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, SVGLength*, RefPtr<SVGLength>, Width, width)
        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, SVGLength*, RefPtr<SVGLength>, Height, height)
        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, int, int, PatternUnits, patternUnits)
        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, int, int, PatternContentUnits, patternContentUnits)
        ANIMATED_PROPERTY_DECLARATIONS(SVGPatternElement, SVGTransformList*, RefPtr<SVGTransformList>, PatternTransform, patternTransform)

        mutable KCanvasImage *m_tile;
        mutable bool m_ignoreAttributeChanges;
        mutable KRenderingPaintServerPattern *m_paintServer;
        
        virtual const SVGElement* contextElement() const { return this; }

    private:
        // notifyAttributeChange helpers:
        void fillAttributesFromReferencePattern(const SVGPatternElement *target, AffineTransform& patternTransformMatrix);
        void drawPatternContentIntoTile(const SVGPatternElement *target, const IntSize &newSize, AffineTransform patternTransformMatrix);
        void notifyClientsToRepaint() const;
    };

} // namespace WebCore

#endif // SVG_SUPPORT
#endif

// vim:ts=4:noet
