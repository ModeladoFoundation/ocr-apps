///////////////////////////////////////////////////////////////////////////////
///
///	\file    ConsolidationStatus.h
///	\author  Paul Ullrich
///	\version February 25, 2013
///
///	<remarks>
///		Copyright 2000-2010 Paul Ullrich
///
///		This file is distributed as part of the Tempest source code package.
///		Permission is granted to use, copy, modify and distribute this
///		source code and its documentation under the terms of the GNU General
///		Public License.  This software is provided "as is" without express
///		or implied warranty.
///	</remarks>

#ifndef _CONSOLIDATIONSTATUS_H_
#define _CONSOLIDATIONSTATUS_H_

#include "DataType.h"
#include "DataLocation.h"

#ifdef USE_MPI
#include "mpi.h"
#endif
#ifdef USE_OCR
typedef int MPI_Request;
#endif
#ifdef USE_OCR_TEST
#include "ocr_vector.hpp"
#else
#include <vector>
#endif

#include <map>

///////////////////////////////////////////////////////////////////////////////

class Grid;

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		A DataType and DataLocation pair.
///	</summary>
class DataTypeLocationPair {

public:
	///	<summary>
	///		Constructor.
	///	</summary>
	DataTypeLocationPair(
		DataType eDataType,
		DataLocation eDataLocation = DataLocation_Node
	) :
		m_eDataType(eDataType),
		m_eDataLocation(eDataLocation)
	{ }

	///	<summary>
	///		Convert this object to an int.
	///	</summary>
	operator int() const {
		return (((int)(m_eDataType) << 2) + (int)(m_eDataLocation));
	}

	///	<summary>
	///		Comparator.
	///	</summary>
	int operator<(const DataTypeLocationPair & pair) const {
		return ((int)(*this) < (int)(pair));
	}

public:
	///	<summary>
	///		DataType of this DataTypeLocationPair.
	///	</summary>
	DataType m_eDataType;

	///	<summary>
	///		DataLocation of this DataTypeLocationPair.
	///	</summary>
	DataLocation m_eDataLocation;
};

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		An array for holding the current consolidation status, which is used
///		for MPI reduce of data to the root processor.
///	</summary>
class ConsolidationStatus {
	friend class Grid;

public:
	typedef std::map<DataTypeLocationPair, int>   ConsolidationIndexMap;
	typedef ConsolidationIndexMap::const_iterator ConsolidationIndexMapIterator;
	typedef ConsolidationIndexMap::value_type     ConsolidationIndexMapPair;

public:
	///	<summary>
	///		Constructor.
	///	</summary>
	ConsolidationStatus(
		const Grid & grid,
#ifdef USE_OCR_TEST
		const Ocr::Vector<DataTypeLocationPair> & vecDataTypes
#else
		const std::vector<DataTypeLocationPair> & vecDataTypes
#endif
	);

public:
	///	<summary>
	///		Generate a MPI_TAG.
	///	</summary>
	static int GenerateTag(
		int ixPatch,
		DataType eDataType,
		DataLocation eDataLocation = DataLocation_Node
	) {
		DataTypeLocationPair key(eDataType, eDataLocation);
		return (ixPatch << 6) + (int)key;
	}

	///	<summary>
	///		Parse the MPI_TAG for DataType and patch index.
	///	</summary>
	static void ParseTag(
		int nTag,
		int & ixPatch,
		DataType & eDataType,
		DataLocation & eDataLocation
	) {
		eDataLocation = static_cast<DataLocation>(nTag & 0x3);
		eDataType = static_cast<DataType>((nTag >> 2) & 0xF);
		ixPatch = (nTag >> 6);
	}

public:
	///	<summary>
	///		Return the number of DataTypes in this structure.
	///	</summary>
	int GetDataTypeCount() const {
		return m_mapDataTypes.size();
	}

	///	<summary>
	///		Determine if the specified DataType and DataLocation is to be
	///		consolidated.
	///	</summary>
	bool Contains(
		DataType eDataType,
		DataLocation eDataLocation = DataLocation_Node
	) const;

	///	<summary>
	///		Determine if all receives have completed.
	///	</summary>
	bool Done() const;

protected:
	/// <summary>
	///		Set the receive status for the given DataType and patch.
	///	</summary>
	void SetReceiveStatus(
		int ixPatch,
		DataType eDataType,
		DataLocation eDataLocation = DataLocation_Node
	);

	///	<summary>
	///		Get the specified send request.
	///	</summary>
	MPI_Request * GetNextSendRequest();

private:
	///	<summary>
	///		Data types stored in this data structure.
	///	</summary>
	ConsolidationIndexMap m_mapDataTypes;

	///	<summary>
	///		Total number of receives set.
	///	</summary>
#ifdef USE_OCR_TEST
	Ocr::Vector32<int> m_vecReceiveStatusCount;
#else
	std::vector<int> m_vecReceiveStatusCount;
#endif

	///	<summary>
	///		Vector of receive status.
	///	</summary>
#ifdef USE_OCR_TEST
	Ocr::Vector32< Ocr::Vector32<bool> > m_vecReceiveStatus;
#else
	std::vector< std::vector<bool> > m_vecReceiveStatus;
#endif

	///	<summary>
	///		Current send request.
	///	</summary>
	int m_nCurrentSendRequest;

	///	<summary>
	///		Vector of send requests.
	///	</summary>
#ifdef USE_OCR_TEST
	Ocr::Vector32<MPI_Request> m_vecSendRequests;
#else
	std::vector<MPI_Request> m_vecSendRequests;
#endif
};

///////////////////////////////////////////////////////////////////////////////

#endif

