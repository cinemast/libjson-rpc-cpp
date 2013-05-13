/**
 * @file json-rpc.h
 * @date 02.08.2011
 * @author Peter Spiess-Knafl
 * @brief json-rpc.h
 *
 * This file is meant to include all necessary .h files of this framework.
 */

#ifndef JSONRPCCPP_H_
#define JSONRPCCPP_H_

#include "server.h"
#include "client.h"

//For error handling and catching Exceptions.
#include "exception.h"

#include "connectors/httpserver.h"
#include "connectors/httpclient.h"

#include "specificationparser.h"
#include "specificationwriter.h"

#endif /* JSONRPCCPP_H_ */
