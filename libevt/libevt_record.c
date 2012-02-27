/*
 * Record functions
 *
 * Copyright (c) 2011-2012, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include <liberror.h>

#include "libevt_definitions.h"
#include "libevt_io_handle.h"
#include "libevt_libbfio.h"
#include "libevt_record.h"
#include "libevt_record_values.h"

/* Initializes the record and its values
 * Returns 1 if successful or -1 on error
 */
int libevt_record_initialize(
     libevt_record_t **record,
     libevt_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     libevt_record_values_t *record_values,
     uint8_t flags,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_initialize";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( *record != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid record value already set.",
		 function );

		return( -1 );
	}
	if( record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record values.",
		 function );

		return( -1 );
	}
	if( ( flags & ~( LIBEVT_RECORD_FLAG_MANAGED_FILE_IO_HANDLE ) ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported flags: 0x%02" PRIx8 ".",
		 function,
		 flags );

		return( -1 );
	}
	internal_record = memory_allocate_structure(
	                 libevt_internal_record_t );

	if( internal_record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create internal record.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_record,
	     0,
	     sizeof( libevt_internal_record_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear internal record.",
		 function );

		memory_free(
		 internal_record );

		return( -1 );
	}
	if( ( flags & LIBEVT_RECORD_FLAG_MANAGED_FILE_IO_HANDLE ) == 0 )
	{
		internal_record->file_io_handle = file_io_handle;
	}
	else
	{
		if( libbfio_handle_clone(
		     &( internal_record->file_io_handle ),
		     file_io_handle,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy file IO handle.",
			 function );

			goto on_error;
		}
		if( libbfio_handle_set_open_on_demand(
		     internal_record->file_io_handle,
		     1,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to set open on demand in file IO handle.",
			 function );

			goto on_error;
		}
	}
	internal_record->io_handle     = io_handle;
	internal_record->record_values = record_values;
	internal_record->flags         = flags;

	*record = (libevt_record_t *) internal_record;

	return( 1 );

on_error:
	if( internal_record != NULL )
	{
		if( ( flags & LIBEVT_RECORD_FLAG_MANAGED_FILE_IO_HANDLE ) != 0 )
		{
			if( internal_record->file_io_handle != NULL )
			{
				libbfio_handle_free(
				 &( internal_record->file_io_handle ),
				 NULL );
			}
		}
		memory_free(
		 internal_record );
	}
	return( -1 );
}

/* Frees a record
 * Returns 1 if successful or -1 on error
 */
int libevt_record_free(
     libevt_record_t **record,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_free";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( *record != NULL )
	{
		internal_record = (libevt_internal_record_t *) *record;
		*record         = NULL;

		/* The io_handle and record_values references are freed elsewhere
		 */
		if( ( internal_record->flags & LIBEVT_RECORD_FLAG_MANAGED_FILE_IO_HANDLE ) != 0 )
		{
			if( internal_record->file_io_handle != NULL )
			{
				if( libbfio_handle_close(
				     internal_record->file_io_handle,
				     error ) != 0 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_CLOSE_FAILED,
					 "%s: unable to close file IO handle.",
					 function );

					return( -1 );
				}
				if( libbfio_handle_free(
				     &( internal_record->file_io_handle ),
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					 "%s: unable to free file IO handle.",
					 function );

					return( -1 );
				}
			}
		}
		memory_free(
		 internal_record );
	}
	return( 1 );
}

/* Retrieves the identifier (record number)
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_identifier(
     libevt_record_t *record,
     uint32_t *identifier,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_identifier";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( identifier == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid identifier.",
		 function );

		return( -1 );
	}
	*identifier = internal_record->record_values->number;

	return( 1 );
}

/* Retrieves the creation time
 * The timestamp is a 32-bit POSIX date and time value
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_creation_time(
     libevt_record_t *record,
     uint32_t *creation_time,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_creation_time";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( creation_time == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid creation time.",
		 function );

		return( -1 );
	}
	*creation_time = internal_record->record_values->creation_time;

	return( 1 );
}

/* Retrieves the written time
 * The timestamp is a 32-bit POSIX date and time value
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_written_time(
     libevt_record_t *record,
     uint32_t *written_time,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_written_time";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( written_time == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid written time.",
		 function );

		return( -1 );
	}
	*written_time = internal_record->record_values->written_time;

	return( 1 );
}

/* Retrieves the event identifier
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_event_identifier(
     libevt_record_t *record,
     uint32_t *event_identifier,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_event_identifier";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( event_identifier == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid event identifier.",
		 function );

		return( -1 );
	}
	*event_identifier = internal_record->record_values->event_identifier;

	return( 1 );
}

/* Retrieves the event type
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_event_type(
     libevt_record_t *record,
     uint16_t *event_type,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_event_type";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( event_type == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid event type.",
		 function );

		return( -1 );
	}
	*event_type = internal_record->record_values->event_type;

	return( 1 );
}

/* Retrieves the size of the UTF-8 encoded source name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf8_source_name_size(
     libevt_record_t *record,
     size_t *utf8_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf8_source_name_size";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_get_utf8_string_size(
	     internal_record->record_values->source_name,
	     0,
	     utf8_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 string size.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the UTF-8 encoded source name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf8_source_name(
     libevt_record_t *record,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf8_source_name";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_copy_to_utf8_string(
	     internal_record->record_values->source_name,
	     0,
	     utf8_string,
	     utf8_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy source name to UTF-8 string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the size of the UTF-16 encoded source name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf16_source_name_size(
     libevt_record_t *record,
     size_t *utf16_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf16_source_name_size";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_get_utf16_string_size(
	     internal_record->record_values->source_name,
	     0,
	     utf16_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 string size.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the UTF-16 encoded source name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf16_source_name(
     libevt_record_t *record,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf16_source_name";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_copy_to_utf16_string(
	     internal_record->record_values->source_name,
	     0,
	     utf16_string,
	     utf16_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy source name to UTF-16 string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the size of the UTF-8 encoded computer name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf8_computer_name_size(
     libevt_record_t *record,
     size_t *utf8_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf8_computer_name_size";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_get_utf8_string_size(
	     internal_record->record_values->computer_name,
	     0,
	     utf8_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 string size.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the UTF-8 encoded computer name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf8_computer_name(
     libevt_record_t *record,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf8_computer_name";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_copy_to_utf8_string(
	     internal_record->record_values->computer_name,
	     0,
	     utf8_string,
	     utf8_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy computer name to UTF-8 string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the size of the UTF-16 encoded computer name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf16_computer_name_size(
     libevt_record_t *record,
     size_t *utf16_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf16_computer_name_size";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_get_utf16_string_size(
	     internal_record->record_values->computer_name,
	     0,
	     utf16_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 string size.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the UTF-16 encoded computer name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf16_computer_name(
     libevt_record_t *record,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf16_computer_name";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_copy_to_utf16_string(
	     internal_record->record_values->computer_name,
	     0,
	     utf16_string,
	     utf16_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy computer name to UTF-16 string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the number of strings
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_number_of_strings(
     libevt_record_t *record,
     int *number_of_strings,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_number_of_strings";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( internal_record->record_values->strings == NULL )
	{
		if( number_of_strings == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			 "%s: invalid number of strings.",
			 function );

			return( -1 );
		}
		*number_of_strings = 0;
	}
	else
	{
		if( libfvalue_value_get_number_of_value_entries(
		     internal_record->record_values->strings,
		     number_of_strings,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve number of strings value entries.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Retrieves the size of a specific UTF-8 encoded string
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf8_string_size(
     libevt_record_t *record,
     int string_index,
     size_t *utf8_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf8_string_size";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( internal_record->record_values->strings == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid string index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_get_utf8_string_size(
	     internal_record->record_values->strings,
	     string_index,
	     utf8_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 string size of strings value entry: %d.",
		 function,
		 string_index );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves a specifig UTF-8 encoded string
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf8_string(
     libevt_record_t *record,
     int string_index,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf8_string";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( internal_record->record_values->strings == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid string index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_copy_to_utf8_string(
	     internal_record->record_values->strings,
	     string_index,
	     utf8_string,
	     utf8_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy strings value entry: %d to UTF-8 string.",
		 function,
		 string_index );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the size of a specific UTF-16 encoded string
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf16_string_size(
     libevt_record_t *record,
     int string_index,
     size_t *utf16_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf16_string_size";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( internal_record->record_values->strings == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid string index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_get_utf16_string_size(
	     internal_record->record_values->strings,
	     string_index,
	     utf16_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 string size of strings value entry: %d.",
		 function,
		 string_index );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves a specifig UTF-16 encoded string
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libevt_record_get_utf16_string(
     libevt_record_t *record,
     int string_index,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     liberror_error_t **error )
{
	libevt_internal_record_t *internal_record = NULL;
	static char *function                     = "libevt_record_get_utf16_string";

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	internal_record = (libevt_internal_record_t *) record;

	if( internal_record->record_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal record - missing record values.",
		 function );

		return( -1 );
	}
	if( internal_record->record_values->strings == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid string index value out of bounds.",
		 function );

		return( -1 );
	}
	if( libfvalue_value_copy_to_utf16_string(
	     internal_record->record_values->strings,
	     string_index,
	     utf16_string,
	     utf16_string_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy strings value entry: %d to UTF-16 string.",
		 function,
		 string_index );

		return( -1 );
	}
	return( 1 );
}

