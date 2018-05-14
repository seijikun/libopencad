/*******************************************************************************
 *  Project: libopencad
 *  Purpose: OpenSource CAD formats support library
 *  Author: Alexandr Borzykh, mush3d at gmail.com
 *  Author: Dmitry Baryshnikov, bishop.dev@gmail.com
 *  Language: C++
 *******************************************************************************
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2016 Alexandr Borzykh
 *  Copyright (c) 2016 NextGIS, <info@nextgis.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *******************************************************************************/
#include "r2000.h"
#include "io.h"
#include "cadgeometry.h"
#include "cadobjects.h"
#include "opencad_api.h"

#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>
#include <cmath>

#ifdef __APPLE__

#include <MacTypes.h>

#endif

#define UNKNOWN1 CADHeader::MAX_HEADER_CONSTANT + 1
#define UNKNOWN2 CADHeader::MAX_HEADER_CONSTANT + 2
#define UNKNOWN3 CADHeader::MAX_HEADER_CONSTANT + 3
#define UNKNOWN4 CADHeader::MAX_HEADER_CONSTANT + 4
#define UNKNOWN5 CADHeader::MAX_HEADER_CONSTANT + 5
#define UNKNOWN6 CADHeader::MAX_HEADER_CONSTANT + 6
#define UNKNOWN7 CADHeader::MAX_HEADER_CONSTANT + 7
#define UNKNOWN8 CADHeader::MAX_HEADER_CONSTANT + 8
#define UNKNOWN9 CADHeader::MAX_HEADER_CONSTANT + 9
#define UNKNOWN10 CADHeader::MAX_HEADER_CONSTANT + 10
#define UNKNOWN11 CADHeader::MAX_HEADER_CONSTANT + 11
#define UNKNOWN12 CADHeader::MAX_HEADER_CONSTANT + 12
#define UNKNOWN13 CADHeader::MAX_HEADER_CONSTANT + 13
#define UNKNOWN14 CADHeader::MAX_HEADER_CONSTANT + 14
#define UNKNOWN15 CADHeader::MAX_HEADER_CONSTANT + 15

CADErrorCodes DXFFileR2000::ReadHeader( OpenOptions eOptions )
{

	return CADErrorCodes::UNSUPPORTED_VERSION;
}

CADErrorCodes DXFFileR2000::ReadClasses( enum OpenOptions eOptions )
{
	return CADErrorCodes::UNSUPPORTED_VERSION;
}

CADErrorCodes DXFFileR2000::CreateFileMap()
{
	return CADErrorCodes::UNSUPPORTED_VERSION;
}

CADObject * DXFFileR2000::GetObject( long dHandle, bool bHandlesOnly )
{
    CADObject * readed_object  = nullptr;

    char   pabyObjectSize[8];
    size_t nBitOffsetFromStart = 0;
    pFileIO->Seek( mapObjects[dHandle], CADFileIO::SeekOrigin::BEG );
    pFileIO->Read( pabyObjectSize, 8 );
    unsigned int dObjectSize = ReadMSHORT( pabyObjectSize, nBitOffsetFromStart );

    // And read whole data chunk into memory for future parsing.
    // + nBitOffsetFromStart/8 + 2 is because dObjectSize doesn't cover CRC and itself.
    size_t             nSectionSize = dObjectSize + nBitOffsetFromStart / 8 + 2;
    unique_ptr<char[]> sectionContentPtr( new char[nSectionSize + 4] );
    char * pabySectionContent = sectionContentPtr.get();
    pFileIO->Seek( mapObjects[dHandle], CADFileIO::SeekOrigin::BEG );
    pFileIO->Read( pabySectionContent, nSectionSize );

    nBitOffsetFromStart = 0;
    dObjectSize         = ReadMSHORT( pabySectionContent, nBitOffsetFromStart );
    short dObjectType = ReadBITSHORT( pabySectionContent, nBitOffsetFromStart );

    if( dObjectType >= 500 )
    {
        CADClass cadClass = oClasses.getClassByNum( dObjectType );
        // FIXME: replace strcmp() with C++ analog
        if( !strcmp( cadClass.sCppClassName.c_str(), "AcDbRasterImage" ) )
        {
            dObjectType = CADObject::IMAGE;
        } else if( !strcmp( cadClass.sCppClassName.c_str(), "AcDbRasterImageDef" ) )
        {
            dObjectType = CADObject::IMAGEDEF;
        } else if( !strcmp( cadClass.sCppClassName.c_str(), "AcDbRasterImageDefReactor" ) )
        {
            dObjectType = CADObject::IMAGEDEFREACTOR;
        } else if( !strcmp( cadClass.sCppClassName.c_str(), "AcDbWipeout" ) )
        {
            dObjectType = CADObject::WIPEOUT;
        }
    }

    // Entities handling
    if( isCommonEntityType( dObjectType ) )
    {
        struct CADCommonED stCommonEntityData; // common for all entities

        stCommonEntityData.nObjectSizeInBits = ReadRAWLONG( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.hObjectHandle     = ReadHANDLE( pabySectionContent, nBitOffsetFromStart );

        short  dEEDSize;
        CADEed dwgEed;
        while( ( dEEDSize = ReadBITSHORT( pabySectionContent, nBitOffsetFromStart ) ) != 0 )
        {
            dwgEed.dLength      = dEEDSize;
            dwgEed.hApplication = ReadHANDLE( pabySectionContent, nBitOffsetFromStart );

            for( short i = 0; i < dEEDSize; ++i )
            {
                dwgEed.acData.push_back( ReadCHAR( pabySectionContent, nBitOffsetFromStart ) );
            }

            stCommonEntityData.aEED.push_back( dwgEed );
        }

        stCommonEntityData.bGraphicsPresented = ReadBIT( pabySectionContent, nBitOffsetFromStart );
        if( stCommonEntityData.bGraphicsPresented )
        {
            size_t nGraphicsDataSize = static_cast<size_t>(ReadRAWLONG( pabySectionContent, nBitOffsetFromStart ));
            // skip read graphics data
            nBitOffsetFromStart += nGraphicsDataSize * 8;
        }
        stCommonEntityData.bbEntMode        = Read2B( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.nNumReactors     = ReadBITLONG( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.bNoLinks         = ReadBIT( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.nCMColor         = ReadBITSHORT( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.dfLTypeScale     = ReadBITDOUBLE( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.bbLTypeFlags     = Read2B( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.bbPlotStyleFlags = Read2B( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.nInvisibility    = ReadBITSHORT( pabySectionContent, nBitOffsetFromStart );
        stCommonEntityData.nLineWeight      = ReadCHAR( pabySectionContent, nBitOffsetFromStart );

        // Skip entitity-specific data, we don't need it if bHandlesOnly == true
        if( bHandlesOnly == true )
        {
            return getEntity( dObjectType, dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );
        }

        switch( dObjectType )
        {
//            case CADObject::BLOCK:
//                return getBlock( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::ELLIPSE:
//                return getEllipse( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::MLINE:
//                return getMLine( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::SOLID:
//                return getSolid( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::POINT:
//                return getPoint( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::POLYLINE3D:
//                return getPolyLine3D( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::RAY:
//                return getRay( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::XLINE:
//                return getXLine( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::LINE:
//                return getLine( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::TEXT:
//                return getText( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//			case CADObject::VERTEX2D:
//				return getVertex2D(dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart);

//            case CADObject::VERTEX3D:
//                return getVertex3D( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::CIRCLE:
//                return getCircle( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::ENDBLK:
//                return getEndBlock( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::POLYLINE2D:
//                return getPolyline2D( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::ATTRIB:
//                return getAttributes( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::ATTDEF:
//                return getAttributesDefn( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::LWPOLYLINE:
//                return getLWPolyLine( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::ARC:
//                return getArc( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::SPLINE:
//                return getSpline( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::POLYLINE_PFACE:
//                return getPolylinePFace( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::IMAGE:
//                return getImage( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::FACE3D:
//                return get3DFace( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::VERTEX_MESH:
//                return getVertexMesh( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::VERTEX_PFACE:
//                return getVertexPFace( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::MTEXT:
//                return getMText( dObjectSize, stCommonEntityData, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::DIMENSION_RADIUS:
//            case CADObject::DIMENSION_DIAMETER:
//            case CADObject::DIMENSION_ALIGNED:
//            case CADObject::DIMENSION_ANG_3PT:
//            case CADObject::DIMENSION_ANG_2LN:
//            case CADObject::DIMENSION_ORDINATE:
//            case CADObject::DIMENSION_LINEAR:
//                return getDimension( dObjectType, dObjectSize, stCommonEntityData, pabySectionContent,
//                                     nBitOffsetFromStart );

//            case CADObject::INSERT:
//                return getInsert( dObjectType, dObjectSize, stCommonEntityData, pabySectionContent,
//                                  nBitOffsetFromStart );

            default:
                return getEntity( dObjectType, dObjectSize, stCommonEntityData, pabySectionContent,
                                  nBitOffsetFromStart );
        }
    } else
    {
        switch( dObjectType )
        {
//            case CADObject::DICTIONARY:
//                return getDictionary( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::LAYER:
//                return getLayerObject( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::LAYER_CONTROL_OBJ:
//                return getLayerControl( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::BLOCK_CONTROL_OBJ:
//                return getBlockControl( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::BLOCK_HEADER:
//                return getBlockHeader( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::LTYPE_CONTROL_OBJ:
//                return getLineTypeControl( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::LTYPE1:
//                return getLineType1( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::IMAGEDEF:
//                return getImageDef( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::IMAGEDEFREACTOR:
//                return getImageDefReactor( dObjectSize, pabySectionContent, nBitOffsetFromStart );

//            case CADObject::XRECORD:
//                return getXRecord( dObjectSize, pabySectionContent, nBitOffsetFromStart );
        }
    }

    return readed_object;
}

CADGeometry * DXFFileR2000::GetGeometry( size_t iLayerIndex, long dHandle, long dBlockRefHandle )
{
    CADGeometry * poGeometry = nullptr;
    unique_ptr<CADEntityObject> readedObject( static_cast<CADEntityObject *>(GetObject( dHandle )) );

    if( nullptr == readedObject )
        return nullptr;

    switch( readedObject->getType() )
    {
        case CADObject::ARC:
        {
            CADArc       * arc    = new CADArc();
            CADArcObject * cadArc = static_cast<CADArcObject *>(
                    readedObject.get());

            arc->setPosition( cadArc->vertPosition );
            arc->setExtrusion( cadArc->vectExtrusion );
            arc->setRadius( cadArc->dfRadius );
            arc->setThickness( cadArc->dfThickness );
            arc->setStartingAngle( cadArc->dfStartAngle );
            arc->setEndingAngle( cadArc->dfEndAngle );

            poGeometry = arc;
            break;
        }

        case CADObject::POINT:
        {
            CADPoint3D     * point    = new CADPoint3D();
            CADPointObject * cadPoint = static_cast<CADPointObject *>(
                    readedObject.get());

            point->setPosition( cadPoint->vertPosition );
            point->setExtrusion( cadPoint->vectExtrusion );
            point->setXAxisAng( cadPoint->dfXAxisAng );
            point->setThickness( cadPoint->dfThickness );

            poGeometry = point;
            break;
        }

        case CADObject::POLYLINE3D:
        {
            CADPolyline3D       * polyline               = new CADPolyline3D();
            CADPolyline3DObject * cadPolyline3D          = static_cast<CADPolyline3DObject *>(
                    readedObject.get());

			polyline->setClosed(cadPolyline3D->bClosed);
			polyline->setSplined(cadPolyline3D->bSplined);

            // TODO: code can be much simplified if CADHandle will be used.
            // to do so, == and ++ operators should be implemented.
            unique_ptr<CADVertex3DObject> vertex;
            long                          currentVertexH = cadPolyline3D->hVertexes[0].getAsLong();
            while( currentVertexH != 0 )
            {
                vertex.reset( static_cast<CADVertex3DObject *>(
                                      GetObject( currentVertexH )) );

                if( vertex == nullptr )
                    break;

                currentVertexH = vertex->stCed.hObjectHandle.getAsLong();
				if (! (vertex->vFlags & 2 || vertex->vFlags & 16)) // ignore tangent / spline frame pts  
					polyline->addVertex( vertex->vertPosition );
                if( vertex->stCed.bNoLinks == true )
                {
                    ++currentVertexH;
                } else
                {
                    currentVertexH = vertex->stChed.hNextEntity.getAsLong( vertex->stCed.hObjectHandle );
                }

                // Last vertex is reached. read it and break reading.
                if( currentVertexH == cadPolyline3D->hVertexes[1].getAsLong() )
                {
                    vertex.reset( static_cast<CADVertex3DObject *>(
                                          GetObject( currentVertexH )) );
					if (!( vertex->vFlags & 2 || vertex->vFlags & 16 )) // ignore tangent / spline frame pts 
						polyline->addVertex( vertex->vertPosition );
                    break;
                }
            }

            poGeometry = polyline;
            break;
        }

        case CADObject::LWPOLYLINE:
        {
            CADLWPolyline       * lwPolyline    = new CADLWPolyline();
            CADLWPolylineObject * cadlwPolyline = static_cast<CADLWPolylineObject *>(
                    readedObject.get());

			lwPolyline->setBulges(cadlwPolyline->adfBulges);
            lwPolyline->setClosed(cadlwPolyline->bClosed );
            lwPolyline->setConstWidth(cadlwPolyline->dfConstWidth );
            lwPolyline->setElevation(cadlwPolyline->dfElevation );
            for( const CADVector& vertex : cadlwPolyline->avertVertexes )
                lwPolyline->addVertex(vertex );
            lwPolyline->setVectExtrusion(cadlwPolyline->vectExtrusion );
            lwPolyline->setWidths( cadlwPolyline->astWidths );

            poGeometry = lwPolyline;
            break;
        }

		case CADObject::POLYLINE2D:
		{
			CADPolyline2D       * polyline2D = new CADPolyline2D();
			CADPolyline2DObject * cadPolyline2D = static_cast<CADPolyline2DObject *>(
                    readedObject.get());
			
			polyline2D->setClosed(cadPolyline2D->bClosed);
			polyline2D->setSplined(cadPolyline2D->bSplined);
			polyline2D->setStartSegWidth(cadPolyline2D->dfStartWidth);
			polyline2D->setEndSegWidth(cadPolyline2D->dfEndWidth);
			polyline2D->setElevation(cadPolyline2D->dfElevation);
			polyline2D->setVectExtrusion(cadPolyline2D->vectExtrusion);

			std::vector<double> bulges;
			vector<pair<double, double> > widths;

			// TODO: code can be much simplified if CADHandle will be used.
			// to do so, == and ++ operators should be implemented.
			unique_ptr<CADVertex2DObject> vertex;
			long                          currentVertexH = cadPolyline2D->hVertexes[0].getAsLong();
			while (currentVertexH != 0)
			{
				vertex.reset(static_cast<CADVertex2DObject *>(
					GetObject(currentVertexH)));

				if (vertex == nullptr)
					break;

				currentVertexH = vertex->stCed.hObjectHandle.getAsLong();
				if ( !( vertex->vFlags & 2 || vertex->vFlags & 16 )) // ignore tangent / spline frame pts  
				{
					polyline2D->addVertex( CADVector( vertex->vertPosition ));
					bulges.push_back( vertex->dfBulge);
					widths.push_back( make_pair(vertex->dfStartWidth, vertex->dfEndWidth ));
				}
				if (vertex->stCed.bNoLinks == true)
				{
					++currentVertexH;
				}
				else
				{
					currentVertexH = vertex->stChed.hNextEntity.getAsLong(vertex->stCed.hObjectHandle);
				}

				// Last vertex is reached. read it and break reading.
				if (currentVertexH == cadPolyline2D->hVertexes[1].getAsLong())
				{
					vertex.reset(static_cast<CADVertex2DObject *>(
						GetObject(currentVertexH)));
					if (!( vertex->vFlags & 2 || vertex->vFlags & 16 )) // ignore tangent / spline frame pts  
					{
						polyline2D->addVertex( CADVector( vertex->vertPosition ));
						widths.push_back( make_pair( vertex->dfStartWidth, vertex->dfEndWidth ));
					}
					break;
				}
			}

			polyline2D->setBulges(bulges);
			polyline2D->setWidths(widths);

			poGeometry = polyline2D;
            break;
		}

        case CADObject::CIRCLE:
        {
            CADCircle       * circle    = new CADCircle();
            CADCircleObject * cadCircle = static_cast<CADCircleObject *>(
                    readedObject.get());

            circle->setPosition( cadCircle->vertPosition );
            circle->setExtrusion( cadCircle->vectExtrusion );
            circle->setRadius( cadCircle->dfRadius );
            circle->setThickness( cadCircle->dfThickness );

            poGeometry = circle;
            break;
        }

        case CADObject::ATTRIB:
        {
            CADAttrib       * attrib    = new CADAttrib();
            CADAttribObject * cadAttrib = static_cast<CADAttribObject *>(
                    readedObject.get() );

            attrib->setPosition( cadAttrib->vertInsetionPoint );
            attrib->setExtrusion( cadAttrib->vectExtrusion );
            attrib->setRotationAngle( cadAttrib->dfRotationAng );
            attrib->setAlignmentPoint( cadAttrib->vertAlignmentPoint );
            attrib->setElevation( cadAttrib->dfElevation );
            attrib->setHeight( cadAttrib->dfHeight );
            attrib->setObliqueAngle( cadAttrib->dfObliqueAng );
            attrib->setPositionLocked( cadAttrib->bLockPosition );
            attrib->setTag( cadAttrib->sTag );
            attrib->setTextValue( cadAttrib->sTextValue );
            attrib->setThickness( cadAttrib->dfThickness );

            poGeometry = attrib;
            break;
        }

        case CADObject::ATTDEF:
        {
            CADAttdef       * attdef    = new CADAttdef();
            CADAttdefObject * cadAttrib = static_cast<CADAttdefObject *>(
                    readedObject.get() );

            attdef->setPosition( cadAttrib->vertInsetionPoint );
            attdef->setExtrusion( cadAttrib->vectExtrusion );
            attdef->setRotationAngle( cadAttrib->dfRotationAng );
            attdef->setAlignmentPoint( cadAttrib->vertAlignmentPoint );
            attdef->setElevation( cadAttrib->dfElevation );
            attdef->setHeight( cadAttrib->dfHeight );
            attdef->setObliqueAngle( cadAttrib->dfObliqueAng );
            attdef->setPositionLocked( cadAttrib->bLockPosition );
            attdef->setTag( cadAttrib->sTag );
            attdef->setTextValue( cadAttrib->sTextValue );
            attdef->setThickness( cadAttrib->dfThickness );
            attdef->setPrompt( cadAttrib->sPrompt );

            poGeometry = attdef;
            break;
        }

        case CADObject::ELLIPSE:
        {
            CADEllipse       * ellipse    = new CADEllipse();
            CADEllipseObject * cadEllipse = static_cast<CADEllipseObject *>(
                    readedObject.get());

            ellipse->setPosition( cadEllipse->vertPosition );
            ellipse->setSMAxis( cadEllipse->vectSMAxis );
            ellipse->setAxisRatio( cadEllipse->dfAxisRatio );
            ellipse->setEndingAngle( cadEllipse->dfEndAngle );
            ellipse->setStartingAngle( cadEllipse->dfBegAngle );

            poGeometry = ellipse;
            break;
        }

        case CADObject::LINE:
        {
            CADLineObject * cadLine = static_cast<CADLineObject *>(
                    readedObject.get());

            CADPoint3D ptBeg( cadLine->vertStart, cadLine->dfThickness );
            CADPoint3D ptEnd( cadLine->vertEnd, cadLine->dfThickness );

            CADLine * line = new CADLine( ptBeg, ptEnd );

            poGeometry = line;
            break;
        }

        case CADObject::RAY:
        {
            CADRay       * ray    = new CADRay();
            CADRayObject * cadRay = static_cast<CADRayObject *>(
                    readedObject.get());

            ray->setVectVector( cadRay->vectVector );
            ray->setPosition( cadRay->vertPosition );

            poGeometry = ray;
            break;
        }

        case CADObject::SPLINE:
        {
            CADSpline       * spline    = new CADSpline();
            CADSplineObject * cadSpline = static_cast<CADSplineObject *>(
                    readedObject.get());

            spline->setScenario( cadSpline->dScenario );
            spline->setDegree( cadSpline->dDegree );
            if( spline->getScenario() == 2 )
            {
                spline->setFitTollerance( cadSpline->dfFitTol );
            } else if( spline->getScenario() == 1 )
            {
                spline->setRational( cadSpline->bRational );
                spline->setClosed( cadSpline->bClosed );
                spline->setWeight( cadSpline->bWeight );
            }
            for( double weight : cadSpline->adfCtrlPointsWeight )
                spline->addControlPointsWeight( weight );

            for( const CADVector& pt : cadSpline->averFitPoints )
                spline->addFitPoint( pt );

            for( const CADVector& pt : cadSpline->avertCtrlPoints )
                spline->addControlPoint( pt );

            poGeometry = spline;
            break;
        }

        case CADObject::TEXT:
        {
            CADText       * text    = new CADText();
            CADTextObject * cadText = static_cast<CADTextObject *>(
                    readedObject.get());

            text->setPosition( cadText->vertInsetionPoint );
            text->setTextValue( cadText->sTextValue );
            text->setRotationAngle( cadText->dfRotationAng );
            text->setObliqueAngle( cadText->dfObliqueAng );
            text->setThickness( cadText->dfThickness );
            text->setHeight( cadText->dfElevation );

            poGeometry = text;
            break;
        }

        case CADObject::SOLID:
        {
            CADSolid       * solid    = new CADSolid();
            CADSolidObject * cadSolid = static_cast<CADSolidObject *>(
                    readedObject.get());

            solid->setElevation( cadSolid->dfElevation );
            solid->setThickness( cadSolid->dfThickness );
            for( const CADVector& corner : cadSolid->avertCorners )
                solid->addCorner( corner );
            solid->setExtrusion( cadSolid->vectExtrusion );

            poGeometry = solid;
            break;
        }

        case CADObject::IMAGE:
        {
            CADImage       * image    = new CADImage();
            CADImageObject * cadImage = static_cast<CADImageObject *>(
                    readedObject.get());

            unique_ptr<CADImageDefObject> cadImageDef( static_cast<CADImageDefObject *>(
                                                               GetObject( cadImage->hImageDef.getAsLong() ) ) );


            image->setClippingBoundaryType( cadImage->dClipBoundaryType );
            image->setFilePath( cadImageDef->sFilePath );
            image->setVertInsertionPoint( cadImage->vertInsertion );
            CADVector imageSize( cadImage->dfSizeX, cadImage->dfSizeY );
            image->setImageSize( imageSize );
            CADVector imageSizeInPx( cadImageDef->dfXImageSizeInPx, cadImageDef->dfYImageSizeInPx );
            image->setImageSizeInPx( imageSizeInPx );
            CADVector pixelSizeInACADUnits( cadImageDef->dfXPixelSize, cadImageDef->dfYPixelSize );
            image->setPixelSizeInACADUnits( pixelSizeInACADUnits );
            image->setResolutionUnits( ( CADImage::ResolutionUnit ) cadImageDef->dResUnits );
            image->setOptions( cadImage->dDisplayProps & 0x08, cadImage->bClipping, cadImage->dBrightness,
                               cadImage->dContrast );
            for( const CADVector& clipPt :  cadImage->avertClippingPolygonVertexes )
            {
                image->addClippingPoint( clipPt );
            }

            poGeometry = image;
            break;
        }

        case CADObject::MLINE:
        {
            CADMLine       * mline    = new CADMLine();
            CADMLineObject * cadmLine = static_cast<CADMLineObject *>(
                    readedObject.get());

            mline->setScale( cadmLine->dfScale );
            mline->setOpened( cadmLine->dOpenClosed == 1 ? true : false );
            for( const CADMLineVertex& vertex : cadmLine->avertVertexes )
                mline->addVertex( vertex.vertPosition );

            poGeometry = mline;
            break;
        }

        case CADObject::MTEXT:
        {
            CADMText       * mtext    = new CADMText();
            CADMTextObject * cadmText = static_cast<CADMTextObject *>(
                    readedObject.get());

            mtext->setTextValue( cadmText->sTextValue );
            mtext->setXAxisAng( cadmText->vectXAxisDir.getX() ); //TODO: is this needed?

            mtext->setPosition( cadmText->vertInsertionPoint );
            mtext->setExtrusion( cadmText->vectExtrusion );

            mtext->setHeight( cadmText->dfTextHeight );
            mtext->setRectWidth( cadmText->dfRectWidth );
            mtext->setExtents( cadmText->dfExtents );
            mtext->setExtentsWidth( cadmText->dfExtentsWidth );

            poGeometry = mtext;
            break;
        }

        case CADObject::POLYLINE_PFACE:
        {
            CADPolylinePFace       * polyline                  = new CADPolylinePFace();
            CADPolylinePFaceObject * cadpolyPface              = static_cast<CADPolylinePFaceObject *>(
                    readedObject.get());

            // TODO: code can be much simplified if CADHandle will be used.
            // to do so, == and ++ operators should be implemented.
            unique_ptr<CADVertexPFaceObject> vertex;
            auto                             dCurrentEntHandle = cadpolyPface->hVertexes[0].getAsLong();
            auto                             dLastEntHandle    = cadpolyPface->hVertexes[1].getAsLong();
            while( true )
            {
                vertex.reset( static_cast<CADVertexPFaceObject *>(
                                      GetObject( dCurrentEntHandle )) );
                /* TODO: this check is excessive, but if something goes wrong way -
             * some part of geometries will be parsed. */
                if( vertex == nullptr )
                    continue;

                polyline->addVertex( vertex->vertPosition );

                /* FIXME: somehow one more vertex which isnot presented is read.
             * so, checking the number of added vertexes */
                /*TODO: is this needed - check on real data
            if ( polyline->hVertexes.size() == cadpolyPface->nNumVertexes )
            {
                delete( vertex );
                break;
            }*/

                if( vertex->stCed.bNoLinks )
                    ++dCurrentEntHandle;
                else
                    dCurrentEntHandle = vertex->stChed.hNextEntity.getAsLong( vertex->stCed.hObjectHandle );

                if( dCurrentEntHandle == dLastEntHandle )
                {
                    vertex.reset( static_cast<CADVertexPFaceObject *>(
                                          GetObject( dCurrentEntHandle )) );
                    polyline->addVertex( vertex->vertPosition );
                    break;
                }
            }

            poGeometry = polyline;
            break;
        }

        case CADObject::XLINE:
        {
            CADXLine       * xline    = new CADXLine();
            CADXLineObject * cadxLine = static_cast<CADXLineObject *>(
                    readedObject.get());

            xline->setVectVector( cadxLine->vectVector );
            xline->setPosition( cadxLine->vertPosition );

            poGeometry = xline;
            break;
        }

        case CADObject::FACE3D:
        {
            CADFace3D       * face      = new CADFace3D();
            CAD3DFaceObject * cad3DFace = static_cast<CAD3DFaceObject *>(
                    readedObject.get());

            for( const CADVector& corner : cad3DFace->avertCorners )
                face->addCorner( corner );
            face->setInvisFlags( cad3DFace->dInvisFlags );

            poGeometry = face;
            break;
        }

        case CADObject::POLYLINE_MESH:
        case CADObject::VERTEX_MESH:
        case CADObject::VERTEX_PFACE_FACE:
        default:
            cerr << "Asked geometry has unsupported type." << endl;
            poGeometry = new CADUnknown();
            break;
    }

    if( poGeometry == nullptr )
        return nullptr;

    // Applying color
    if( readedObject->stCed.nCMColor == 256 ) // BYLAYER CASE
    {
        CADLayer& oCurrentLayer = this->GetLayer( iLayerIndex );
        poGeometry->setColor( CADACIColors[oCurrentLayer.getColor()] );
    }
    else if( readedObject->stCed.nCMColor <= 255 &&
             readedObject->stCed.nCMColor >= 0 ) // Excessive check until BYBLOCK case will not be implemented
    {
        poGeometry->setColor( CADACIColors[readedObject->stCed.nCMColor] );
    }

    // Applying EED
    // Casting object's EED to a vector of strings
    vector<string> asEED;
    for( auto      citer     = readedObject->stCed.aEED.cbegin(); citer != readedObject->stCed.aEED.cend(); ++citer )
    {
        string sEED = "";
        // Detect the type of EED entity
        switch( citer->acData[0] )
        {
            case 0: // string
            {
                unsigned char nStrSize = citer->acData[1];
                // +2 = skip CodePage, no idea how to use it anyway
                for( size_t   i        = 0; i < nStrSize; ++i )
                {
                    sEED += citer->acData[i + 4];
                }
                break;
            }
            case 1: // invalid
            {
                DebugMsg( "Error: EED obj type is 1, error in R2000::getGeometry()" );
                break;
            }
            case 2: // { or }
            {
                sEED += citer->acData[1] == 0 ? '{' : '}';
                break;
            }
            case 3: // layer table ref
            {
                // FIXME: get CADHandle and return getAsLong() result.
                sEED += "Layer table ref (handle):";
                for( size_t i = 0; i < 8; ++i )
                {
                    sEED += citer->acData[i + 1];
                }
                break;
            }
            case 4: // binary chunk
            {
                unsigned char nChunkSize = citer->acData[1];
                sEED += "Binary chunk (chars):";
                for( size_t i = 0; i < nChunkSize; ++i )
                {
                    sEED += citer->acData[i + 2];
                }
                break;
            }
            case 5: // entity handle ref
            {
                // FIXME: get CADHandle and return getAsLong() result.
                sEED += "Entity handle ref (handle):";
                for( size_t i = 0; i < 8; ++i )
                {
                    sEED += citer->acData[i + 1];
                }
                break;
            }
            case 10:
            case 11:
            case 12:
            case 13:
            {
                sEED += "Point: {";
                double dfX = 0, dfY = 0, dfZ = 0;
                memcpy( & dfX, citer->acData.data() + 1, 8 );
                memcpy( & dfY, citer->acData.data() + 9, 8 );
                memcpy( & dfZ, citer->acData.data() + 17, 8 );
                sEED += to_string( dfX );
                sEED += ';';
                sEED += to_string( dfY );
                sEED += ';';
                sEED += to_string( dfZ );
                sEED += '}';
                break;
            }
            case 40:
            case 41:
            case 42:
            {
                sEED += "Double:";
                double dfVal = 0;
                memcpy( & dfVal, citer->acData.data() + 1, 8 );
                sEED += to_string( dfVal );
                break;
            }
            case 70:
            {
                sEED += "Short:";
                short dVal = 0;
                memcpy( & dVal, citer->acData.data() + 1, 2 );
                sEED += to_string( dVal );
                break;
            }
            case 71:
            {
                sEED += "Long Int:";
                long dVal = 0;
                memcpy( & dVal, citer->acData.data() + 1, 4 );
                sEED += to_string( dVal );
                break;
            }
            default:
            {
                DebugMsg( "Error in parsing geometry EED: undefined typecode: %d", ( int ) citer->acData[0] );
            }
        }
        asEED.emplace_back( sEED );
    }

    // Getting block reference attributes.
    if( dBlockRefHandle != 0 )
    {
        vector<CADAttrib>           blockRefAttributes;
        unique_ptr<CADInsertObject> spoBlockRef( static_cast<CADInsertObject *>( GetObject( dBlockRefHandle ) ) );

        if( spoBlockRef->hAttribs.size() != 0 )
        {
            long dCurrentEntHandle = spoBlockRef->hAttribs[0].getAsLong();
            long dLastEntHandle    = spoBlockRef->hAttribs[0].getAsLong();

            while( spoBlockRef->bHasAttribs )
            {
                // FIXME: memory leak, somewhere in CAD* destructor is a bug
                CADEntityObject * attDefObj = static_cast<CADEntityObject *>(
                        GetObject( dCurrentEntHandle, true ) );

                if( dCurrentEntHandle == dLastEntHandle )
                {
                    if( attDefObj == nullptr )
                        break;

                    CADAttrib * attrib = static_cast<CADAttrib *>(
                            GetGeometry( iLayerIndex, dCurrentEntHandle ) );

                    if( attrib )
                    {
                        blockRefAttributes.push_back( CADAttrib( * attrib ) );
                        delete attrib;
                    }
                    delete attDefObj;
                    break;
                }

                if( attDefObj != nullptr )
                {
                    if( attDefObj->stCed.bNoLinks )
                        ++dCurrentEntHandle;
                    else
                        dCurrentEntHandle = attDefObj->stChed.hNextEntity.getAsLong( attDefObj->stCed.hObjectHandle );

                    CADAttrib * attrib = static_cast<CADAttrib *>(
                            GetGeometry( iLayerIndex, dCurrentEntHandle ) );

                    if( attrib )
                    {
                        blockRefAttributes.push_back( CADAttrib( * attrib ) );
                        delete attrib;
                    }
                    delete attDefObj;
                } else
                {
                    assert ( 0 );
                }
            }
            poGeometry->setBlockAttributes( blockRefAttributes );
        }
    }

    poGeometry->setEED( asEED );
    return poGeometry;
}













DXFFileR2000::DXFFileR2000( CADFileIO * poFileIO ) : CADFile( poFileIO )
{
	oHeader.addValue( CADHeader::OPENCADVER, static_cast<int>(CADVersions::DXF_R2000) );
}

DXFFileR2000::~DXFFileR2000()
{
}

CADErrorCodes DXFFileR2000::ReadSectionLocators()
{
	return CADErrorCodes::UNSUPPORTED_VERSION;
}

CADDictionary DXFFileR2000::GetNOD()
{

}










CADEntityObject* DXFFileR2000::getEntity( int dObjectType, long dObjectSize, CADCommonED stCommonEntityData,
										   const char * pabyInput, size_t& nBitOffsetFromStart )
{
	CADEntityObject * entity = new CADEntityObject();

	//FIXME

	return entity;
}
