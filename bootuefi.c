#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    Print(L"Hola mundo desde un bootloader UEFI!\n");

    while(1); // evitar que termine

    return EFI_SUCCESS;
}