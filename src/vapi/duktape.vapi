[CCode (lower_case_cprefix = "duk_", cheader_filename = "duktape.h")]
namespace Duktape {

	[SimpleType]
  [CCode (cname = "duk_int_t", has_type_id = false)]
  public struct Int : int {}

	[SimpleType]
  [CCode (cname = "duk_size_t", has_type_id = false)]
  public struct Size : size_t {}

	[SimpleType]
  [CCode (cname = "duk_bool_t", has_type_id = false)]
  public struct Bool : bool {}

	[SimpleType]
  [CCode (cname = "duk_double_t", has_type_id = false)]
  public struct Double : double {}

	[SimpleType]
  [CCode (cname = "duk_idx_t", has_type_id = false)]
  public struct Index : int {}

	[SimpleType]
  [CCode (cname = "duk_idx_t", has_type_id = false)]
  public struct ArrayIndex : int {}

	[CCode (cname = "duk_int_t", has_type_id = false, cprefix = "DUK_TYPE_")]
	public enum Type {
		NONE,
		UNDEFINED,
		NULL,
		BOOLEAN,
		NUMBER,
		STRING,
		OBJECT,
		BUFFER,
		POINTER,
		LIGHTFUNC,
	}

	[CCode (cname = "duk_int_t", has_type_id = false)]
	public enum Exec {
		SUCCESS = 0,
	}

	[CCode (cname = "duk_ret_t", has_type_id = false, cprefix = "DUK_RET_")]
	public enum Return {
		ERROR,
		EVAL_ERROR,
		RANGE_ERROR,
		REFERENCE_ERROR,
		SYNTAX_ERROR,
		TYPE_ERROR,
		URI_ERROR,
	}

	[CCode (cname = "duk_errcode_t", has_type_id = false, cprefix = "DUK_ERR_")]
	public enum Error {
		NONE,
		ERROR,
		EVAL_ERROR,
		RANGE_ERROR,
		REFERENCE_ERROR,
		SYNTAX_ERROR,
		TYPE_ERROR,
		URI_ERROR,
	}

	[CCode (cname = "duk_uint_t", has_type_id = false, cprefix = "DUK_BUFOBJ_")]
	public enum BufferObject {
		NODEJS_BUFFER,
		ARRAYBUFFER,
		DATAVIEW,
		INT8ARRAY,
		UINT8ARRAY,
		UINT8CLAMPEDARRAY,
		INT16ARRAY,
		UINT16ARRAY,
		INT32ARRAY,
		UINT32ARRAY,
		FLOAT32ARRAY,
		FLOAT64ARRAY,
	}

	[CCode (cname = "duk_uint_t", has_type_id = false, cprefix = "DUK_ENUM_")]
	public enum Enum {
		INCLUDE_NONENUMERABLE,
		INCLUDE_HIDDEN,
		INCLUDE_SYMBOLS,
		EXCLUDE_STRINGS,
		OWN_PROPERTIES_ONLY,
		ARRAY_INDICES_ONLY,
		SORT_ARRAY_INDICES,
		NO_PROXY_BEHAVIOR,
	}

	[CCode (cname = "duk_c_function", has_target = false)]
	public delegate Return CFunction (Duktape vm);

	[SimpleType]
	[CCode (cname = "duk_function_list_entry", has_type_id = false)]
	public struct FunctionListEntry {
		public unowned string key;
		public CFunction value;
		public Int nargs;
	}

	[Compact]
	[CCode (free_function = "duk_destroy_heap", cname = "duk_context", cprefix = "duk_")]
	public class Duktape {

		[CCode (cname = "duk_create_heap_default")]
		public Duktape.default ();

		// compile
		public Exec pcompile_string_filename (uint flags, string src);

		// call
		public Exec pcall (Index nargs);

		// put object
		public Bool put_global_string (string key);
		public Index push_object ();

		// get object
		public Bool get_global_string (string key);
		public void require_object (Index idx);
		public Bool get_prop_string (Index idx, string key);
		public Bool get_prop_index (Index obj, ArrayIndex idx);
		public void enum (Index obj, Enum flags);
		public Bool next (Index enum_idx, Bool get_value);

		// get array
		public Bool is_array (Index idx);

		// put error
		public Index push_error_object (Error code, string fmt);
	
		// put string
		public void push_string (string str);

		// get string
		public unowned string get_string (Index idx);
		public unowned string get_string_default (Index idx, string def);
		public unowned string require_string (Index idx);
		public unowned string safe_to_string (Index idx);
		public unowned string safe_to_stacktrace (Index idx);

		// get number
		public Double get_number (Index idx);
		public Double get_number_default (Index idx, Double def);
		public Int to_int (Index idx);

		// get number
		public Bool get_boolean (Index idx);
		public Bool get_boolean_default (Index idx, Bool def);

		// put function
		public Index push_c_function (CFunction func, Index nargs);
		public void put_function_list (Index index, [CCode (array_length = false, array_null_terminated = true)] FunctionListEntry[] funcs);

		// put buffer
		public void* push_fixed_buffer (Size size);
		public void push_external_buffer ();
		public void config_buffer (Index idx, uint8[] data);
		public void push_buffer_object (Index idx, Size offset, Size length, BufferObject flags);

		// get buffer
		public unowned uint8[] require_buffer_data (Index idx);

		// put pointer
		public void push_pointer (void* ptr);

		// get pointer
		public unowned void* require_pointer (Index idx);

		// stack
		public void pull (Index idx);
		public void dup (Index idx);
		public void pop ();
		public void pop_2 ();
		public void pop_3 ();
		public void remove (Index idx);
		public Index get_top ();
		public Type get_type (Index idx);

		// misc
		public Return throw ();
		public Size get_length (Index idx);
	}

}
