<?php

$target_path = "";

$target_path = $target_path . basename( $_FILES['picture']['name']); 

if(move_uploaded_file($_FILES['picture']['tmp_name'], $target_path)) {  
            echo "1";
        }else{
            print_r($_FILES);
        }
?>