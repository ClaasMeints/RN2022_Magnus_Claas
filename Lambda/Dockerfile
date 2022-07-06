FROM public.ecr.aws/lambda/python:3.7

RUN pip3 install --upgrade pip

RUN pip3 install pillow --no-cache-dir
RUN pip3 install https://raw.githubusercontent.com/alexeygrigorev/serverless-deep-learning/master/tflite/tflite_runtime-2.2.0-cp37-cp37m-linux_x86_64.whl --no-cache-dir

COPY lite-model_aiy_vision_classifier_birds_V1_3.tflite model.tflite
COPY lambda_function.py lambda_function.py

CMD [ "lambda_function.lambda_handler" ]