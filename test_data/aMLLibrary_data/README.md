## Quick Tutorial on aMLLibrary
As we already stated, aMLLibrary can be used by network_butcher to generate and import the weights in the block graph.
This package can be used to both perform the training of machine learning models and to perform inference.

## Why is aMLLibrary used?

aMLLibrary is used by the weight importer class block_aMLLibrary_Weight_Importer to generate and import the weights
produced by the models of [[1]](#1) in the block graph.

In particular, the general idea of block_aMLLibrary_Weight_Importer (that is detailed by [[1]](#1) in his thesis) is to 
estimate the execution time of each layer in a Deep Neural Network on two devices (an edge device and a cloud device) by 
constructing a collection of regression models that should, a priori, estimate the layer execution time.
The training data for these models is obtained by running a profiler application on several networks.
The profiler will:
1) Construct the linear graph of the network (the linearized graph produced by Contrained_Block_Graph_Builder with mode 
'output')
2) Measure the average time required to execute each 'node' of the linearized graph in the edge and cloud devices. 
General informations about the executed layers is also saved (the output tensor length, the MACs of the layers, the number of 
parameters and the number of executed layers as well as the time required to send the output tensor from the edge device to the 
cloud device)

Once a profiler application has produced the final result, the regression model for the chosen device can be computed in two steps:
1) Prepare a configuration file with the parameters for the training (see the example in the config/test1 and 
config/test2 folders).
Here, a user can specify the allowed machine learning techniques to use, the parameters related to the training procedure,
as well as the information related to the input CSV file (the columns containing the features and the variable to predict).
2) Run the training procedure by executing the following command in the aMLLibrary directory:
```
python3 run.py -c config.ini -o output_directory
```

where config.ini is the configuration file and output_directory is the directory where the results will be stored.
In particular, the directory will store the best model it managed to find as a .pickle file.

To perform the prediction using the previously generated model, we have to follow two steps:
1) Prepare a configuration file with the parameters for the inference (see the example in the config/predict_0.ini and
and config/predict_1.ini files).
2) Run the inference procedure by executing the following command in the aMLLibrary directory:
```
python3 predict.py -c predict_config.ini -r path_to_model -o prediction_folder
```
where predict_config.ini is the configuration file, path_to_model is the path to the model to use for the prediction
and prediction_folder is the directory where the results will be stored.

Network Butcher will only execute the prediction part by generating, given the proper parameters and the regression models,
the correct predict configuration files and by calling the 'predict' method of the 'Predictor' class.
The output CSV files will then be read using a Csv_Weight_Importer.

Since the model training is a time-consuming process, we provide a set of pre-trained models (available in the
models folder) that can be used to perform the inference. The models were trained using the CSV file in the 'inputs' folder
(provided by [[1]](#1)) and the configuration files in the 'config' folder.

## References
<a id="1">[1]</a>
Eduard Ionut, Chirica (2022).
'An approach and a tool for the performance profiling and prediction of partitioned Deep Neural Networks in Computing Continua environments'
