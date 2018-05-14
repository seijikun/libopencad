#pragma once

#include "cadfile.h"
#include "opencad_api.h"

class DXFFileR2000 : public CADFile
{
public:
	DXFFileR2000( CADFileIO * poFileIO );
	virtual             ~DXFFileR2000() override;

protected:
	virtual CADErrorCodes ReadSectionLocators() override;
	virtual CADErrorCodes ReadHeader( enum OpenOptions eOptions ) override;
	virtual CADErrorCodes ReadClasses( enum OpenOptions eOptions ) override;
	virtual CADErrorCodes CreateFileMap() override;

    CADObject   * GetObject( long dHandle, bool bHandlesOnly = false ) override;
    CADGeometry * GetGeometry( size_t iLayerIndex, long dHandle, long dBlockRefHandle = 0 ) override;

    CADDictionary GetNOD() override;
protected:
//    CADBlockObject           * getBlock( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CADEllipseObject         * getEllipse( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                           size_t& nBitOffsetFromStart );
//    CADSolidObject           * getSolid( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CADPointObject           * getPoint( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CADPolyline3DObject      * getPolyLine3D( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                              size_t& nBitOffsetFromStart );
//    CADRayObject             * getRay( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                       size_t& nBitOffsetFromStart );
//    CADXLineObject           * getXLine( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CADLineObject            * getLine( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                        size_t& nBitOffsetFromStart );
//    CADTextObject            * getText( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                        size_t& nBitOffsetFromStart );
//	CADVertex2DObject        * getVertex2D(long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//											size_t& nBitOffsetFromStart);
//    CADVertex3DObject        * getVertex3D( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                            size_t& nBitOffsetFromStart );
//    CADCircleObject          * getCircle( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                          size_t& nBitOffsetFromStart );
//    CADEndblkObject          * getEndBlock( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                            size_t& nBitOffsetFromStart );
//    CADPolyline2DObject      * getPolyline2D( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                              size_t& nBitOffsetFromStart );
//    CADAttribObject          * getAttributes( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                              size_t& nBitOffsetFromStart );
//    CADAttdefObject          * getAttributesDefn( long dObjectSize, CADCommonED stCommonEntityData,
//                                                  const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADLWPolylineObject      * getLWPolyLine( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                              size_t& nBitOffsetFromStart );
//    CADArcObject             * getArc( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                       size_t& nBitOffsetFromStart );
//    CADSplineObject          * getSpline( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                          size_t& nBitOffsetFromStart );
	CADEntityObject          * getEntity( int dObjectType, long dObjectSize, CADCommonED stCommonEntityData,
										  const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADInsertObject          * getInsert( int dObjectType, long dObjectSize, CADCommonED stCommonEntityData,
//                                          const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADDictionaryObject      * getDictionary( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADXRecordObject         * getXRecord( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADLayerObject           * getLayerObject( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADLayerControlObject    * getLayerControl( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADBlockControlObject    * getBlockControl( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADBlockHeaderObject     * getBlockHeader( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADLineTypeControlObject * getLineTypeControl( long dObjectSize, const char * pabyInput,
//                                                   size_t& nBitOffsetFromStart );
//    CADLineTypeObject        * getLineType1( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADMLineObject           * getMLine( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CADPolylinePFaceObject   * getPolylinePFace( long dObjectSize, CADCommonED stCommonEntityData,
//                                                 const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADImageObject           * getImage( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CAD3DFaceObject          * get3DFace( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                          size_t& nBitOffsetFromStart );
//    CADVertexMeshObject      * getVertexMesh( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                              size_t& nBitOffsetFromStart );
//    CADVertexPFaceObject     * getVertexPFace( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                               size_t& nBitOffsetFromStart );
//    CADDimensionObject       * getDimension( short dObjectType, long dObjectSize, CADCommonED stCommonEntityData,
//                                             const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADMTextObject           * getMText( long dObjectSize, CADCommonED stCommonEntityData, const char * pabyInput,
//                                         size_t& nBitOffsetFromStart );
//    CADImageDefObject        * getImageDef( long dObjectSize, const char * pabyInput, size_t& nBitOffsetFromStart );
//    CADImageDefReactorObject * getImageDefReactor( long dObjectSize, const char * pabyInput,
//                                                   size_t& nBitOffsetFromStart );
//    void                     fillCommonEntityHandleData( CADEntityObject * pEnt, const char * pabyInput,
//                                                         size_t& nBitOffsetFromStart );
};
