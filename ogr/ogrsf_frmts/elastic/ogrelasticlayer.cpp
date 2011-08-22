/******************************************************************************
 * $Id $
 *
 * Project:  Elastic Search Translator
 * Purpose:  
 * Author:   
 *
 ******************************************************************************
 * Copyright (c) 2008, Adam Estrada
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "ogr_elastic.h"
#include "cpl_conv.h"
#include "cpl_minixml.h"
#include "ogr_api.h"
#include "ogr_p.h"

/************************************************************************/
/*                            OGRElasticLayer()                          */
/************************************************************************/

OGRElasticLayer::OGRElasticLayer (const char *pszFilename,
				  const char *pszLayerName,
				  OGRElasticDataSource * poDS,
				  OGRSpatialReference * poSRSIn,
				  int bWriteMode)
{
  this->pszLayerName = CPLStrdup (pszLayerName);
  this->poDS = poDS;

  // If we are overwriting, then delete the current index if it exists
  if (poDS->bOverwrite)
    {
      poDS->
	DeleteIndex (CPLSPrintf ("%s/%s", poDS->GetName (), pszLayerName));
    }

  // Create the index
  poDS->UploadFile (CPLSPrintf ("%s/%s", poDS->GetName (), pszLayerName), "");

  // If we have a user specified mapping, then go ahead and update it now
  if (poDS->psMapping != NULL)
    {
      poDS->
	UploadFile (CPLSPrintf
		    ("%s/%s/Features/_mapping", poDS->GetName (),
		     pszLayerName), poDS->psMapping);
    }

  poFeatureDefn = new OGRFeatureDefn (pszLayerName);
  poFeatureDefn->Reference ();

  poSRS = poSRSIn;
  if (poSRS)
    poSRS->Reference ();

  nTotalFeatureCount = 0;

  poFeature = NULL;
  ResetReading ();
  return;
}

/************************************************************************/
/*                            ~OGRElasticLayer()                            */
/************************************************************************/

OGRElasticLayer::~OGRElasticLayer ()
{
  PushIndex ();
  //poFeatureDefn->Release();

  if (poSRS != NULL)
    poSRS->Release ();

  if (poFeature)
    delete poFeature;
}


/************************************************************************/
/*                            GetLayerDefn()                            */
/************************************************************************/

OGRFeatureDefn *
OGRElasticLayer::GetLayerDefn ()
{
  return poFeatureDefn;
}

/************************************************************************/
/*                            ResetReading()                            */
/************************************************************************/

void
OGRElasticLayer::ResetReading ()
{
  return;
}

/************************************************************************/
/*                           GetNextFeature()                           */
/************************************************************************/

OGRFeature *
OGRElasticLayer::GetNextFeature ()
{
  CPLError (CE_Failure, CPLE_NotSupported,
	    "Cannot read features when writing a Elastic file");
  return NULL;
}

/************************************************************************/
/*                           GetSpatialRef()                            */
/************************************************************************/

OGRSpatialReference *
OGRElasticLayer::GetSpatialRef ()
{
  return poSRS;
}

/************************************************************************/
/*                           CreateFeature()                            */
/************************************************************************/

CPLString::size_type repl (CPLString & s,
			   const CPLString & from, const CPLString & to)
{
  CPLString::size_type cnt (CPLString::npos);

  if (from != to && !from.empty ())
    {
      CPLString::size_type pos1 (0);
      CPLString::size_type pos2 (0);
      const CPLString::size_type from_len (from.size ());
      const CPLString::size_type to_len (to.size ());
      cnt = 0;

      while ((pos1 = s.find (from, pos2)) != CPLString::npos)
	{
	  s.replace (pos1, from_len, to);
	  pos2 = pos1 + to_len;
	  ++cnt;
	}
    }

  return cnt;
}

CPLString
OGRElasticLayer::BuildMap ()
{
  CPLString location =
    "\"geometry\": {\"properties\": {\"coordinates\": { \"store\": \"yes\", \"type\": \"geo_point\"}}},";
  location += sFields.c_str ();
  return CPLString ("{\"Feature\" :{\"properties\" : {") + location + "}}}";
}

OGRErr
OGRElasticLayer::CreateFeature (OGRFeature * poFeature)
{

  // Check to see if the user has elected to only write out the mapping file
  // This method will only write out one layer from the vector file in cases where there are multiple layers
  if (poDS->psWriteMap != NULL)
    {
      if (!sFields.empty ())
	{
	  CPLString map = BuildMap ();
	  sFields = "";
	  FILE *f = fopen (poDS->psWriteMap, "wb");
	  if (f)
	    {
	      fwrite (map.c_str (), 1, map.length (), f);
	      fclose (f);
	    }
	}
      return OGRERR_NONE;
    }

  // Check to see if we have any fields to upload to this index
  if (poDS->psMapping == NULL && sFields.length () != 0)
    {
      poDS->
	UploadFile (CPLSPrintf
		    ("%s/%s/Feature/_mapping", poDS->GetName (),
		     pszLayerName), BuildMap ());
      sFields = "";
    }

  // Get the center point of the geometry
  OGREnvelope env;
  poFeature->GetGeometryRef ()->getEnvelope (&env);
  CPLString fields =
    CPLSPrintf
    ("\"geometry\": {\"type\": \"POINT\", \"coordinates\": [%f,%f]}",
     (env.MaxX + env.MinX) * 0.5, (env.MaxY + env.MinY) * 0.5);
  // For every field that
  int fieldCount = poFeatureDefn->GetFieldCount ();
  for (int i = 0; i < fieldCount; i++)
    {
      switch (poFeatureDefn->GetFieldDefn (i)->GetType ())
	{
	case OFTInteger:
	  fields += CPLSPrintf (", \"%s\" : %d",
				poFeatureDefn->GetFieldDefn (i)->
				GetNameRef (),
				poFeature->GetFieldAsInteger (poFeatureDefn->
							      GetFieldDefn
							      (i)->
							      GetNameRef ()));
	  break;
	case OFTReal:
	  fields += CPLSPrintf (", \"%s\" : %f",
				poFeatureDefn->GetFieldDefn (i)->
				GetNameRef (),
				poFeature->GetFieldAsDouble (poFeatureDefn->
							     GetFieldDefn
							     (i)->
							     GetNameRef ()));
	  break;
	default:
	  {
	    CPLString tmp =
	      poFeature->GetFieldAsString (poFeatureDefn->GetFieldDefn (i)->
					   GetNameRef ());

	    // Need to make sure the json is safe from any foreign characters
	    repl (tmp, "'", "''");
	    repl (tmp, "\\", "\\\\");
	    repl (tmp, "\"", "\\\"");
	    repl (tmp, "\b", "\\b");
	    repl (tmp, "\f", "\\f");
	    repl (tmp, "\n", "\\n");
	    repl (tmp, "\r", "\\r");
	    repl (tmp, "\t", "\\t");

	    fields += CPLSPrintf (", \"%s\" : \"%s\"",
				  poFeatureDefn->GetFieldDefn (i)->
				  GetNameRef (), tmp.c_str ());
	  }
	}
    }
  nTotalFeatureCount++;

  // Check to see if we're using bulk uploading
  if (poDS->nBulkUpload > 0)
    {
      sIndex +=
	CPLSPrintf
	("{\"index\" :{\"_index\":\"%s\", \"_type\":\"Feature\"}}\n{%s}\n\n",
	 pszLayerName, fields.c_str ());

      // Only push the data if we are over our bulk upload limit
      if ((int) sIndex.length () > poDS->nBulkUpload)
	{
	  PushIndex ();
	}

    }
  else
    {				// Fall back to using single item upload for every feature
      poDS->
	UploadFile (CPLSPrintf
		    ("%s/%s/Feature/", poDS->GetName (), pszLayerName),
		    CPLString ("{\"Feature\" :{") + fields + "}}");
    }

  return OGRERR_NONE;
}

void
OGRElasticLayer::PushIndex ()
{
  if (sIndex.empty ())
    {
      return;
    }
  poDS->UploadFile (CPLSPrintf ("%s/_bulk", poDS->GetName ()), sIndex);

  sIndex.clear ();
}

/************************************************************************/
/*                            CreateField()                             */
/************************************************************************/

OGRErr
OGRElasticLayer::CreateField (OGRFieldDefn * poFieldDefn, int bApproxOK)
{

  if (!sFields.empty ())
    {
      sFields += ",";
    }

  switch (poFieldDefn->GetType ())
    {
    case OFTInteger:
      sFields +=
	CPLSPrintf (" \"%s\": { \"store\": \"yes\", \"type\": \"integer\"}",
		    poFieldDefn->GetNameRef ());
      break;
    case OFTReal:
      sFields +=
	CPLSPrintf (" \"%s\": { \"store\": \"yes\", \"type\": \"float\"}",
		    poFieldDefn->GetNameRef ());
      break;
    case OFTString:
      sFields +=
	CPLSPrintf (" \"%s\": { \"store\": \"yes\", \"type\": \"string\"}",
		    poFieldDefn->GetNameRef ());
      break;
      //case OFTTime:
    case OFTDateTime:
    case OFTDate:
      sFields +=
	CPLSPrintf
	(" \"%s\": { \"store\": \"yes\", \"format\": \"yyyy/MM/dd HH:mm:ss||yyyy/MM/dd\", \"type\": \"date\"}",
	 poFieldDefn->GetNameRef ());
      break;
    default:
      sFields +=
	CPLSPrintf (" \"%s\": { \"store\": \"yes\", \"type\": \"string\"}",
		    poFieldDefn->GetNameRef ());
/*		OFTIntegerList = 1,
		OFTRealList = 3,
		OFTStringList = 5,
		OFTWideString = 6,
		OFTWideStringList = 7,
		OFTBinary = 8,
		OFTMaxType = 11*/
    }

  poFeatureDefn->AddFieldDefn (poFieldDefn);
  return OGRERR_NONE;
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int
OGRElasticLayer::TestCapability (const char *pszCap)
{
  if (EQUAL (pszCap, OLCFastFeatureCount))
    return FALSE;

  else if (EQUAL (pszCap, OLCStringsAsUTF8))
    return TRUE;

  else if (EQUAL (pszCap, OLCSequentialWrite))
    return TRUE;
  else
    return FALSE;
}

/************************************************************************/
/*                          GetFeatureCount()                           */
/************************************************************************/

int
OGRElasticLayer::GetFeatureCount (int bForce)
{
  CPLError (CE_Failure, CPLE_NotSupported,
	    "Cannot read features when writing a Elastic file");
  return 0;
}
