<?php

require_once '../core/api_exception.class.php';

throw new APIException('Some weird error');

echo "Some other output that should not be printed";

?>
