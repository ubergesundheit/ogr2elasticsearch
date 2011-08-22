/******************************************************************************
 * $Id $
 *
 * Project:  ElasticSearch Translator
 * Purpose:
 * Author:
 *
 ******************************************************************************
 * Copyright (c) 2011, Adam Estrada
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
#include "cpl_string.h"
#include "cpl_csv.h"

//#ifdef HAVE_CURL
#include <curl/curl.h>
//#endif


/************************************************************************/
/*                          OGRElasticDataSource()                          */
/************************************************************************/

OGRElasticDataSource::OGRElasticDataSource ()
{
  papoLayers = NULL;
  nLayers = 0;
  pszName = NULL;
  bUseExtensions = FALSE;
  bWriteHeaderAndFooter = TRUE;
  pCurl = curl_easy_init ();
}

/************************************************************************/
/*                         ~OGRElasticDataSource()                          */
/************************************************************************/

OGRElasticDataSource::~OGRElasticDataSource ()
{
  for (int i = 0; i < nLayers; i++)
    delete papoLayers[i];
  CPLFree (papoLayers);
  CPLFree (pszName);
  curl_easy_cleanup (pCurl);
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int
OGRElasticDataSource::TestCapability (const char *pszCap)
{
  if (EQUAL (pszCap, ODsCCreateLayer))
    return TRUE;
  else if (EQUAL (pszCap, ODsCDeleteLayer))
    return TRUE;
  else
    return FALSE;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

OGRLayer *
OGRElasticDataSource::GetLayer (int iLayer)
{
  if (iLayer < 0 || iLayer >= nLayers)
    return NULL;
  else
    return papoLayers[iLayer];
}

/************************************************************************/
/*                            CreateLayer()                             */
/************************************************************************/

OGRLayer *
OGRElasticDataSource::CreateLayer (const char *pszLayerName,
				   OGRSpatialReference * poSRS,
				   OGRwkbGeometryType eType,
				   char **papszOptions)
{
/*    if (poSRS != NULL)
    {
        OGRSpatialReference oSRS;
        oSRS.SetWellKnownGeogCS("WGS84");
        if (poSRS->IsSame(&oSRS) == FALSE)
        {
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Only WGS84 SRS is supported");
            return NULL;
        }
    }
*/
  nLayers++;
  papoLayers =
    (OGRElasticLayer **) CPLRealloc (papoLayers,
				     nLayers * sizeof (OGRElasticLayer *));
  papoLayers[nLayers - 1] =
    new OGRElasticLayer (pszName, pszLayerName, this, poSRS, TRUE);

  return papoLayers[nLayers - 1];
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

int
OGRElasticDataSource::Open (const char *pszFilename, int bUpdateIn)
{
  CPLError (CE_Failure, CPLE_NotSupported,
	    "OGR/Elastic driver does not support opening a file");
  return FALSE;
}


/************************************************************************/
/*                               Create()                               */
/************************************************************************/

struct WriteThis
{
  const char *readptr;
  int sizeleft;
};

static size_t
UploadStringCallback (void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct WriteThis *pooh = (struct WriteThis *) userp;

  if (size * nmemb < 1)
    return 0;

  if (pooh->sizeleft)
    {
      *(char *) ptr = pooh->readptr[0];	/* copy one single byte */
      pooh->readptr++;		/* advance pointer */
      pooh->sizeleft--;		/* less data left */
      return 1;			/* we return 1 byte at a time! */
    }

  return 0;			/* no more data left to deliver */
}

static size_t
WriteCallback (void *ptr, size_t size, size_t nmemb, void *data)
{
  /* we are not interested in the downloaded bytes itself,
     so we only return the size we would have saved ... */
  (void) ptr;			/* unused */
  (void) data;			/* unused */
  return (size_t) (size * nmemb);
}

void
OGRElasticDataSource::DeleteIndex (const CPLString & url)
{
//#ifdef HAVE_CURL
  CURL *curl = (CURL *) pCurl;
  CURLcode res;

  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  //curl_easy_setopt(curl, CURLOPT_STDERR, myNullFile);
  //curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  //curl_easy_setopt(curl, CURLOPT_MUTE, 1);

  curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
  curl_easy_setopt (curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  res = curl_easy_perform (curl);
  curl_easy_reset (curl);
//#endif
}

void
OGRElasticDataSource::UploadFile (const CPLString & url,
				  const CPLString & data)
{
//#ifdef HAVE_CURL
  CURL *curl = (CURL *) pCurl;
  CURLcode res;

  struct WriteThis pooh;

  pooh.readptr = data.c_str ();
  pooh.sizeleft = (int) data.length ();

  curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());

  /* Now specify we want to POST data */
  curl_easy_setopt (curl, CURLOPT_POST, 1L);

  /* we want to use our own read function */
  curl_easy_setopt (curl, CURLOPT_READFUNCTION, UploadStringCallback);

  /* pointer to pass to our read function */
  curl_easy_setopt (curl, CURLOPT_READDATA, &pooh);

  static int globaldebug = 0;
  int *debug = &globaldebug;

  if (*debug == 1)
    {
      /* get verbose debug output please */
      curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
    }
  else
    {
      /* send all data to this function  */
      curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    }

  //curl_easy_setopt(curl, CURLOPT_STDERR, NULL);
  //curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  //curl_easy_setopt(curl, CURLOPT_MUTE, 1);

  curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, (curl_off_t) pooh.sizeleft);

  /* Perform the request, res will get the return code */
  res = curl_easy_perform (curl);
  if (CURLE_OK != res)
    {
      res = res;
    }

  /* always cleanup */
  //curl_easy_cleanup(curl);
//#endif
}

int
OGRElasticDataSource::Create (const char *pszFilename, char **papszOptions)
{
  this->psMapping = NULL;
  this->psWriteMap = CPLGetConfigOption ("ES_WRITEMAP", NULL);
  this->psMetaFile = CPLGetConfigOption ("ES_META", NULL);
  this->bOverwrite = (int) CPLAtof (CPLGetConfigOption ("ES_OVERWRITE", "0"));
  this->nBulkUpload = (int) CPLAtof (CPLGetConfigOption ("ES_BULK", "0"));

  // Read in the meta file from disk
  if (this->psMetaFile != NULL)
    {
      int fsize;
      char *fdata;
      FILE *fp;

      fp = fopen (this->psMetaFile, "rb");
      if (fp != NULL)
	{
	  fseek (fp, 0, SEEK_END);
	  fsize = (int) ftell (fp);

	  fdata = (char *) malloc (fsize + 1);

	  fseek (fp, 0, SEEK_SET);
	  fread (fdata, fsize, 1, fp);
	  fdata[fsize] = 0;
	  this->psMapping = fdata;
	}
    }

  curl_easy_setopt (this->pCurl, CURLOPT_URL,
		    CPLSPrintf ("%s/_status", pszFilename));
  curl_easy_setopt (this->pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
  CURLcode res = curl_easy_perform (this->pCurl);
  if (CURLE_OK != res)
    {
      CPLError (CE_Failure, CPLE_NoWriteAccess,
		"Could not connect to server");
      return FALSE;
    }
  pszName = CPLStrdup (pszFilename);
  return TRUE;
}
