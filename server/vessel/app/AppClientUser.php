<?php

namespace App;

use Illuminate\Database\Eloquent\Model;

class AppClientUser extends Model
{

    public function client()
		{
				return $this->belongsTo('App\AppClient', 'client_id', 'client_id');
		}

		public function user()
		{
				return $this->belongsTo('App\User', 'user_id', 'user_id');
		}

		protected $table = 'app_client_user';
		protected $fillable = ['client_id', 'user_id'];

}
