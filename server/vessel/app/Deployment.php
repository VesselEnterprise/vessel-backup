<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class Deployment extends Model
{

		//32 chars
		public static function generateToken()
		{
				return bin2hex(random_bytes(16));
		}

		protected $dates = [
			'created_at', 'updated_at', 'expires_at'
		];

    protected $table = 'deployment';
}
