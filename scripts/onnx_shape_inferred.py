import onnx

base_name = 'mobilenet_v2'
model_path_no_ext = '../models/' + base_name
ext_name = '.onnx'
model_path = model_path_no_ext + ext_name
inferred_model_path = model_path_no_ext + '-inferred' + ext_name


# output the inferred model to the specified model path
onnx.shape_inference.infer_shapes_path(model_path, inferred_model_path)