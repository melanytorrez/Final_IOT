import json
import boto3

iot_client = boto3.client('iot-data')
THING_NAME = 'ControlSala'

def lambda_handler(event, context):
    payload = {
        "state": {
            "desired": {
                "luz_sala": "OFF" # <-- LA ÚNICA DIFERENCIA
            }
        }
    }
    try:
        iot_client.update_thing_shadow(
            thingName=THING_NAME,
            payload=json.dumps(payload)
        )
        return {'statusCode': 200, 'body': 'Light OFF command sent!'}
    except Exception as e:
        print(f"Error: {e}")
        raise e