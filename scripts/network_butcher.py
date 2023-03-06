from aMLLibrary.model_building.predictor import Predictor

def network_butcher(regressor_file, output_folder, config_file):
    predictor_obj = Predictor(regressor_file=regressor_file, output_folder=output_folder, debug=False)
    predictor_obj.predict(config_file=config_file, mape_to_file=False)