#include "header.h"
#include "vtsystem.h"
extern "C" NTSTATUS DriverEntry(_In_ struct _DRIVER_OBJECT *DriverObject, _In_ PUNICODE_STRING RegisterPath);
void DriverUnload(_DRIVER_OBJECT * DriverObject);

extern "C" NTSTATUS DriverEntry(_In_ struct _DRIVER_OBJECT *DriverObject,_In_ PUNICODE_STRING RegisterPath)
{
	NTSTATUS status = STATUS_SUCCESS;

	DriverObject->DriverUnload = DriverUnload;
	KdPrint(("Exit DriverEntry.\n"));
	StartVirtualTechnology();
	return status;
}

void DriverUnload(_DRIVER_OBJECT * DriverObject)
{
	KdPrint(("Exit DriverUnload.\n"));
	StopVirtualTechnology();
}