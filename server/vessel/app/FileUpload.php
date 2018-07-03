<?php

namespace App;

use Illuminate\Database\Eloquent\Model;
use App\File;
use Spatie\BinaryUuid\HasBinaryUuid;

class FileUpload extends File
{
	use HasBinaryUuid;

	public function getKeyName()
	{
			return 'upload_id';
	}

	public function file() {
		return $this->belongsTo('App\File', 'file_id', 'file_id');
	}

	public function parts() {
		return $this->hasMany('App\FileUploadPart', 'upload_id', 'upload_id');
	}

	protected $table = 'file_upload';

}
