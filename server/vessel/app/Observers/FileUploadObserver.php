<?php

namespace App\Observers;

use App\FileUpload;
use Carbon\Carbon;

class FileUploadObserver
{
    /**
     * Handle the file upload "created" event.
     *
     * @param  \App\FileUpload  $fileUpload
     * @return void
     */
    public function created(FileUpload $fileUpload)
    {
        //
    }

    /**
     * Handle the file upload "updated" event.
     *
     * @param  \App\FileUpload  $fileUpload
     * @return void
     */
    public function updated(FileUpload $fileUpload)
    {
        //If the Upload has been completed, mark the associated file as uploaded
				if ( $fileUpload->uploaded ) {
					$file = $fileUpload->file;
					$file->uploaded = 1;
					$file->last_backup = Carbon::now();
					$file->save();
				}

    }

    /**
     * Handle the file upload "deleted" event.
     *
     * @param  \App\FileUpload  $fileUpload
     * @return void
     */
    public function deleted(FileUpload $fileUpload)
    {
        //
    }

    /**
     * Handle the file upload "restored" event.
     *
     * @param  \App\FileUpload  $fileUpload
     * @return void
     */
    public function restored(FileUpload $fileUpload)
    {
        //
    }

    /**
     * Handle the file upload "force deleted" event.
     *
     * @param  \App\FileUpload  $fileUpload
     * @return void
     */
    public function forceDeleted(FileUpload $fileUpload)
    {
        //
    }
}
