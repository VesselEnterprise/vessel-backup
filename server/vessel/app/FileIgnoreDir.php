<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class FileIgnoreDir extends Model
{
	protected $fillable = ['file_type'];
	protected $table = 'file_ignore_dir';
}
