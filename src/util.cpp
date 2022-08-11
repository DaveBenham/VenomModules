#include "util.hpp"

json_t* json_bool(bool value){
	return value ? json_true() : json_false();
}

json_t* json_vecArray(Vec * array, int length){
	json_t *jArray = json_array();
	for(int i = 0; i < length; i++){
		json_t *jVec = json_object();
		json_object_set_new(jVec, "x", json_real(array[i].x));
		json_object_set_new(jVec, "y", json_real(array[i].y));
		json_array_insert_new(jArray, i, jVec);
	}
	return jArray;
}
void json_vecArray_value(json_t* jArray, Vec * array, int length){
	for(int i = 0; i < length; i++){
		json_t *jVec = json_array_get(jArray, i);
		float x = json_real_value(json_object_get(jVec,"x"));
		float y = json_real_value(json_object_get(jVec,"y"));
		array[i] = Vec(x,y);
	}
}


json_t* json_floatArray(float * array, int length){
	json_t *jArray = json_array();
	for(int i = 0; i < length; i++){
		json_array_insert_new(jArray, i, json_real(array[i]));
	}
	return jArray;
}
void json_floatArray_value(json_t* jArray, float * array, int length){
	for(int i = 0; i < length; i++){
		array[i] = json_real_value(json_array_get(jArray, i));
	}
}

json_t* json_boolArray(bool * array, int length){
	json_t *jArray = json_array();
	for(int i = 0; i < length; i++){
		json_array_insert_new(jArray, i, json_bool(array[i]));
	}
	return jArray;
}
void json_boolArray_value(json_t* jArray, bool * array, int length){
	for(int i = 0; i < length; i++){
		array[i] = json_is_true(json_array_get(jArray, i));
	}
}

float mod_0_max(float val, float max){
	int whole = std::floor(val/max);
	return val - whole * max;
}

int mod_0_max(int val, int max){
	int whole = std::floor((float)val/max);
	return val - whole * max;
}