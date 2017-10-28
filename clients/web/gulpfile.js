var gulp = require('gulp'),
    request = require('request'),
    fs = require('fs'),
    connect = require('gulp-connect'),
	fileinclude = require('gulp-file-include');
	
var src_dir = "src/";
var build_dir = "build/";

	
gulp.task('html', function() {
  gulp.src(src_dir + '*.htm')
	.pipe(fileinclude({
		prefix: '@@',
		basepath: '@file'
	}))
    .pipe(gulp.dest('build'))
    .pipe(connect.reload());
});
 
gulp.task('appcache', function() {
  gulp.src(src_dir + '*.appcache')
    .pipe(gulp.dest('build'))
    .pipe(connect.reload());
});
 
 
gulp.task('connect', function() {
    connect.server({
		root: 'build',
        livereload: true
    });
});

gulp.task('watch', function() {
    gulp.watch(src_dir + '*.htm', ['html']);
    gulp.watch(src_dir + 'js/*.js', ['html']);
    gulp.watch(src_dir + '*.appcache', ['appcache']);
});


gulp.task('upload', ['html', 'appcache'], function() {
	var url = 'http://192.168.0.40/edit';
	var options = {
		url: url,
		headers: {
			'Content-Type': 'multipart/form-data'
		}
	};
	
	var r = request.post(options, function optionalCallback(err, httpResponse, body) {
			if (err) {
				return console.error('upload failed:', err);
			}
			console.log('Upload successful!  Server responded with:', body);
		});
	var form = r.form();
	form.append('data', fs.createReadStream(__dirname + "/" + build_dir + '/index.htm'), {filename: '/index.htm', contentType: "application/octet-stream"});
	form.append('data', fs.createReadStream(__dirname + "/" + build_dir + '/offline.appcache'), {filename: '/offline.appcache', contentType: "application/octet-stream"});
});

gulp.task('default', ['html', 'appcache']);
gulp.task('serve', ['watch', 'connect']);