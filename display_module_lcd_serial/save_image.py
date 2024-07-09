import os
import sys
from PIL import Image

def convert_image_to_raw(input_image_path, output_directory):
    # Obtener el nombre base del archivo de entrada (sin la ruta)
    base_name = os.path.basename(input_image_path)
    # Cambiar la extensión a .raw
    raw_name = os.path.splitext(base_name)[0] + ".raw"
    # Crear la ruta completa del archivo de salida en el directorio de salida
    output_raw_path = os.path.join(output_directory, raw_name)

    # Abrir la imagen
    image = Image.open(input_image_path).convert('1')  # Convertir a blanco y negro (1 bit por píxel)
    
    # Redimensionar la imagen a 32x32 si no es de ese tamaño
    image = image.resize((32, 32), Image.ANTIALIAS)

    # Convertir la imagen a datos en bruto
    raw_data = image.tobytes()

    # Guardar los datos en el archivo de salida
    with open(output_raw_path, "wb") as f:
        f.write(raw_data)

    print(f"Imagen convertida y guardada en {output_raw_path}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: python3 save_image.py <ruta_de_la_imagen>")
        sys.exit(1)
    
    input_image_path = sys.argv[1]
    output_directory = "data"
    
    # Crear la carpeta 'data' si no existe
    os.makedirs(output_directory, exist_ok=True)
    
    # Convertir la imagen y guardar el archivo RAW
    convert_image_to_raw(input_image_path, output_directory)
