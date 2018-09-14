<?php

namespace App;

use Laravel\Scout\Searchable;
use Illuminate\Database\Eloquent\Model;

class Setting extends Model
{
    use Searchable;

		function toSearchableArray() {
			return [
				'setting_id' => $this->setting_id,
				'setting_name' => $this->setting_name,
				'display_name' => $this->display_name,
				'description' => $this->description,
				'category' => $this->category
			];
		}

    protected $table = 'setting';
		protected $primaryKey = 'setting_id';
}
