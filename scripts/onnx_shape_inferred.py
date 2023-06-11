import onnx
import argparse

parser = argparse.ArgumentParser(prog="Onnx Shape Inferrer")
parser.add_argument('-i', '--input', type=str, help='Input path of the .onnx model')
parser.add_argument('-o', '--output', type=str, help='Output path of the .onnx model')

args = parser.parse_args()

# output the inferred model to the specified model path
onnx.shape_inference.infer_shapes_path(args.input, args.output)