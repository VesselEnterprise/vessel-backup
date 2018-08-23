@extends('layouts.app')

@section ('scripts')
	<script src="{{ asset('js/tablesort.min.js') }}"></script>
	<script>
		$(document).ready( function() {
			$('table').tablesort();
		});
	</script>
@endsection

@section('content')

	<div class="ui very padded container">
		<h2 class="ui header">
			Files
			<div class="sub header">View and Manage files</div>
		</h2>
	</div>

	<div class="ui very padded container">
		<table class="ui selectable sortable stackable padded striped table">
			<thead>
				<tr>
					<th class="no-sort" colspan="8">
		      	{{ $files->links() }}
		    	</th>
			  </tr>
				<tr>
					<th class="no-sort"></th>
					<th>Filename</th>
					<th>User</th>
					<th>File Type</th>
					<th>File Size</th>
					<th>Storage Provider</th>
					<th>Hash</th>
					<th>Last Backup</th>
				</tr>
			</thead>
			<tbody>
				@foreach ($files as $file)
					<tr>
						<td>
							<div class="ui fitted checkbox">
			          <input type="checkbox"><label></label>
			        </div>
						</td>
						<td><a href="{{ route('file.show', $file->file_id_text) }}">{{ $file->file_name }}</a></td>
						<td>{{ $file->user_id }}</td>
						<td>{{ $file->file_type }}</td>
						<td>{{ $file->file_size }}</td>
						<td>AWS</td>
						<td>{{ $file->hash }}</td>
						<td>{{ $file->last_backup }}</td>
					</tr>
				@endforeach
			</tbody>
			<tfoot>
		    <tr>
					<th colspan="8">
						<div class="ui">
							<button class="ui primary button">
								<i class="icon download"></i>
							  Download
							</button>
							<button class="red ui button">
								<i class="icon delete"></i>
							  Delete
							</button>
						</div>
			      {{ $files->links() }}
		    	</th>
			  </tr>
			</tfoot>
		</table>
	</div>

@endsection
