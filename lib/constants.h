#pragma once

enum class CADVersions
{
	UNKNOWN   = 0,

    DWG_R13   = 1012,
    DWG_R14   = 1014,
    DWG_R2000 = 1015,
    DWG_R2004 = 1018,
    DWG_R2007 = 1021,
    DWG_R2010 = 1024,
    DWG_R2013 = 1027,
    DXF_UNDEF = -1000,
    DXF_R13   = -DWG_R13,
    DXF_R14   = -DWG_R14,
    DXF_R2000 = -DWG_R2000,
    DXF_R2004 = -DWG_R2004,
    DXF_R2007 = -DWG_R2007,
    DXF_R2010 = -DWG_R2010,
    DXF_R2013 = -DWG_R2013
};

enum class CADErrorCodes
{
    SUCCESS = 0, /**< operation successfully executed */
    FILE_OPEN_FAILED, /**< failed to open CAD file */
    UNSUPPORTED_VERSION, /**< unsupported CAD file version */
    FILE_PARSE_FAILED, /**< failed to parse file */
    SECTION_LOCATOR_READ_FAILED, /**< failed to read section locator */
    HEADER_SECTION_READ_FAILED, /**< failed to read header section */
    CLASSES_SECTION_READ_FAILED, /**< failed to read classes section */
    TABLES_SECTION_READ_FAILED, /**< failed to read tables section */
    BLOCKS_SECTION_READ_FAILED, /**< failed to read blocks section */
    ENTITIES_SECTION_READ_FAILED, /**< failed to read entities section */
    OBJECTS_SECTION_READ_FAILED, /**< failed to read objects section */
    THUMBNAILIMAGE_SECTION_READ_FAILED, /**< failed to read thumbnailimage section */
    TABLE_READ_FAILED, /**< failed to read table*/
    VALUE_EXISTS                    /**< the value already exist in the header */
};
