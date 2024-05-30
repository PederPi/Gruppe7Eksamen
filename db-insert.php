<?php
ini_set('display_errors', 1); // set to 0 for production version
error_reporting(E_ALL); // Report all PHP errors

$db_servername = "localhost";
$db_username = "debian-sys-maint";
$db_password = "joxUak45bS7zOZ5X";
$db_name = "my_database1";

// check if the password matches
if($_POST['password'] == "Kode1234!"){
        // connection to your database : new mysqli(host, username, password, dbname, port, socket)
        $mysqli = new mysqli($db_servername ,$db_username, $db_password, $db_name);
        if ($mysqli -> connect_errno){
                echo "Failed to connect to MySQL: " . $mysqli -> connect_error;
                exit();
        }

        // temperature
        echo "POST: temperature values: ".$_POST['temperature_value']. ",". $_POST ['temperature_dateTime']."\r\n";

        // insert values to database
        $mysqli -> query("INSERT INTO `temperature` (`value`, `measurement_date`) VALUES ('".$_POST ['temperature_value']."', '".$_POST['temperature_dateTim>

        // Print auto-generated id
        echo "temperature: New record has id: " . $mysqli -> insert_id."\r\n";

        // image
        $img_target_dir = "data-rpi/images/";
        $img_target_file = $img_target_dir . basename($_FILES["file-image"]["name"]);
        $img_uploadOk = 1;
        $img_imageFileType = strtolower(pathinfo($img_target_file,PATHINFO_EXTENSION));

        // Check if image file is a actual image or fake image
        $check = getimagesize($_FILES["file-image"]["tmp_name"]);
        if($check !== false){
                echo "File is an image - " . $check["mime"] . ".";
                $img_uploadOk = 1;
        }else{
                echo "File is not an image.";
                $img_uploadOk = 0;
        }
        // Check if file already exists
        if (file_exists($img_target_file)){
                echo "Sorry, file already exists.";
                $uploadOk = 0;
        }
        // Allow certain file formats
        if($img_imageFileType != "jpg" && $img_imageFileType != "png" && $img_imageFileType != "jpeg" ){
                echo "Sorry, only JPG, JPEG, PNG files are allowed.";
                $img_uploadOk = 0;
        }
        // Check if $img_uploadOk is set to 0 by an error
        if ($img_uploadOk == 0){
                echo "Sorry, your file was not uploaded.";
        }else{ //if everything is ok, try to upload file.
                if (move_uploaded_file($_FILES["file-image"]["tmp_name"], $img_target_file)){
                        echo "The file ". htmlspecialchars( basename( $_FILES["file-image"]["name"])). " has been uploaded\r\n";
                        // take the photo file name and "." replace ":" to get the date in a database-appropriate format.
                        //$file_creation_date = str_replace('.', ':', htmlspecialchars( basename( $_FILES["fileimage"]["name"])));
                        // insert values to database
                        $mysqli -> query("INSERT INTO `photo` (`file_name`, `creation_date`) VALUES ('".htmlspecialchars( basename( $_FILES["file-image"]["n>
                        // Print auto-generated id
                        echo "photo: New record has id: " . $mysqli -> insert_id;
                }else{
                        echo "Sorry, there was an error uploading your file.";
                }
        }
        // close db connection
        $mysqli -> close();
}else{
        echo "incorrect password";
}
?>
