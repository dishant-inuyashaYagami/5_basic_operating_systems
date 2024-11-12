bool
get_bool_env(const char* var_name, bool def_val)
{
    const char* var_val = getenv(var_name);
    if (!var_val) {
        return def_val;
    }
    if (strcmp(var_val,"true") == 0 || strcmp(var_val,"True") == 0 || strcmp(var_val,"1") == 0 || strcmp(var_val,"TRUE") == 0){
	return true;
    }
    return def_val;
}