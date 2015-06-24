#ifndef IRESPONSE_H
#define IRESPONSE_H

#ifdef _WIN32
#pragma once
#endif

class IResponse
{
public:
	virtual void ServerResponded(void) = 0;
	virtual void ServerFailedToRespond(void) = 0;
};

#endif
